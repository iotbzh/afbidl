#!/usr/bin/guile -s 
!#
(use-modules (json))

; compute the C presentation
(include "c-tools.scm")
(include "logging.scm")
(include "jsonschema.scm")

; get the arguments
(define in-file (cadr (command-line)))
(dolog 1 "processing file " in-file "\n")

; open the input JSON file
(dolog 1 "opening file " in-file " ... ")
(define in-port (open-input-file in-file))
(if (not (input-port? in-port))
   (begin
      (print "\nerror! cant read " in-file "\n")
      (exit 1)
      ))
(dolog 1 "done\n")

; process the json input
(dolog 1 "reading file " in-file " ... ")
(define in-desc (json->scm in-port))
(close-port in-port)
(dolog 1 "done\n")

; show the input as SCM
(dolog 1 "SCM: " in-desc "\n")

; show the input as JSON
(dolog 1 "JSON: " (scm->json in-desc #:pretty #t) "\n")

; compute the C presentation
(define out-C-column 120)
(define out-C-tabs   "   ")
(define in-desc-C (C-string-fmt (scm->json-string in-desc #:pretty #f) out-C-tabs (- out-C-column 1 (string-length out-C-tabs))))
(dolog 1 "SCM: " in-desc-C "\n\n")

; manage references
(define (string-positions-full str char start end)
   (if (>= start end)
      '()
      (let ((end (string-positions-full str char (+ start 1) end)))
         (if (eq? (string-ref str start) char)
            (cons start end) end))))
(define (string-split-full str char start end)
   (letrec ((f (lambda (s l)
         (if (and (pair? l) (pair? (cdr l)))
            (let ((b (+ (car l) 1))
               (e (cadr l))
               (r (f s (cdr l))))
               (if (< b e) (cons (string-copy s b e) r) r))
            '()))))
   (f str (cons (- start 1) (append (string-positions-full str char start end) (list end))))))
(define (refstr->list x) (string-split-full x #\/ 2 (string-length x)))
(define ($ref->list x) (refstr->list (cdar x)))
(define (is-$ref? x)
   (and
      (pair? x)
      (pair? (car x))
      (equal? (caar x) "$ref")
      (let ((y (cdar x)))
         (and
            (string? y)
            (>= (string-length y) 2)
            (eq? (string-ref y 0) #\#)
            (eq? (string-ref y 1) #\/)))))
(define (search-desc root path)
   (cond
      ((not (list? path)) #f)
      ((eq? path '()) root)
      ((list? root)
         (let ((a (assoc ((lambda (x) (if (symbol? x) (symbol->string x) x)) (car path)) root)))
            (if a (search-desc (cdr a) (cdr path)) #f)))
      ((and (vector? root) (number? (car path)) (<= 0 (car path)) (< (car path) (vector-length root)))
         (search-desc (vector-ref root (car path)) (cdr path)))
      (else #f)))
(define (explore-desc root fun)
   (letrec ((expdesc (lambda (node path fun)
         (fun node path)
         (cond
            ((pair? node)
               (do ((i node (cdr i)))
                   ((not (pair? i)))
                   (when (pair? (car i)) (expdesc (cdar i) (cons (caar i) path) fun))))
            ((vector? node)
               (do ((n (vector-length node))
                    (i 0 (+ i 1)))
                   ((>= i n))
                   (expdesc (vector-ref node i) (cons i path) fun)))
         ))))
      (expdesc root '() fun)))

; create list of paths and list of cross references
(define desc-refs '())
(define path-refs '())
(explore-desc in-desc 
   (lambda (node path)
      (unless (eq? node (search-desc in-desc  (reverse path)))
         (write path)
         (write " ==>> ")
         (write node)
         (newline))
      (when (is-$ref? node)
         (let* (
            (p (reverse path))
            (r ($ref->list node))
            (a (assoc r desc-refs))
         )
            (set! path-refs (cons (cons p r) path-refs))
            (if a
               (set-cdr! a (cons p (cdr a)))
               (set! desc-refs (cons `(,r ,p) desc-refs)))))))
(iflog 2
   (for-each (lambda (x) (print "FOUND REF " (car x) " FROM PATHS " (cdr x) "\n")) desc-refs)
   (newline)
   (for-each (lambda (x) (print "PATH " (car x) " REFS " (cdr x) "\n")) path-refs)
   (newline))

; check references exist
(for-each (lambda (x)
   (unless (search-desc in-desc (car x))
      (for-each display (list "error! undefined ref: " x " <<<"  ">>>\n"))))
   desc-refs)

; check recusive references
(define rec-refs '())
(define (ref-is-rec? ref)
   (letrec (
      (prefix? (lambda (r p) 
         (cond
            ((eq? r '()) #t)
            ((eq? p '()) #f)
            ((equal? (car r) (car p)) (prefix? (cdr r) (cdr p)))
            (else #f))))
      (yadd (lambda (r pr c olpr)
;(print "yadd R=" r "\n")
;(print "yadd PR=" pr "\n")
;(print "yadd C=" c "\n")
;(print "\n")
         (if (and (prefix? r (car pr)) (not (member (cdr pr) c)))
            (xadd (cdr pr) olpr (cons (cdr pr) c) olpr)
            c)))
      (xadd (lambda (r lpr c olpr)
;(print "xadd R=" r "\n")
;(print "xadd LPR=" lpr "\n")
;(print "xadd C=" c "\n")
;(print "\n")
         (if (pair? lpr)
            (xadd r (cdr lpr) (yadd r (car lpr) c olpr) olpr)
            c)))
   )
   (let ((z (xadd ref path-refs '() path-refs)))
;      (print ref "->" z "<-\n")
      (member ref z))
   ))
(for-each (lambda (x)
   (if (ref-is-rec? (car x))
      (set! rec-refs (cons (car x) rec-refs))))
   desc-refs)
(for-each (lambda (ref) (print "recursive ref: " ref "\n")) rec-refs)
(newline)

;; expand the refences recursively
(define (expand-desc root)
   (letrec ((expand (lambda (node path)
         (cond
            ((is-$ref? node)
               (expand (search-desc root ($ref->list node)) path))
            ((and (pair? node) (pair? (car node)) (string? (caar node)))
               (let* ((n (caar node))
                      (v (expand (cdar node) (cons n path)))
                      (r (expand (cdr node) path)))
                  (cons (cons n v) r)))
            ((vector? node)
               (do ((n (vector-length node))
                    (v (make-vector (vector-length node)))
                    (i 0 (+ i 1)))
                   ((>= i n) v)
                   (vector-set! v i (expand (vector-ref node i) (cons i path)))))
            (else node)
         ))))
      (expand root '())))

;(print "BEFORE\n" (scm->json-string in-desc #:pretty #t) "\n")
;(print "AFTER\n" (scm->json-string (expand-desc in-desc) #:pretty #t) "\n")

; some variables
(define def-gen-vars '(
   (preinit      "#/tools/afb-genskel/preinit")
   (init    "#/tools/afb-genskel/init")
   (onevent      "#/tools/afb-genskel/onevent")
   (scope   "#/tools/afb-genskel/scope"   "static")
   (prefix       "#/tools/afb-genskel/prefix"       "afb_verb_")
   (postfix      "#/tools/afb-genskel/postfix"      "_cb")
   (provideclass "#/tools/afb-genskel/provide-class")
   (requireclass "#/tools/afb-genskel/require-class")
   (requireapi   "#/tools/afb-genskel/require-api")
   (priv    "#/tools/afb-genskel/private"       #f)
   (noconc       "#/tools/afb-genskel/noconcurrency" #f)
   (api     "#/info/apiname")
   (title   "#/info/title"            "?")
   (desc    "#/info/description")
))
(define (init-vars defs root)
   (map (lambda (d)
      (let ((v (search-desc root (refstr->list (cadr d)))))
         (cons (car d) (if v v (if (pair? (cddr d)) (caddr d) (cddr d))))))
      defs))
(define current-vars (init-vars def-gen-vars in-desc))
(define (var x) (cdr (assq x current-vars)))
(define (set-var! x v) (set-cdr! (assq x current-vars) v))
(dolog 2 "VARS: " current-vars "\n")
(set-var! 'scope (let ((c (var 'scope))) (if (string-null? c) c (string-append (string-trim c) " "))))
(dolog 2 "VARS: " current-vars "\n")

; enumerate items of an object
(define (for-each-object-item node fun) (if (list? node) (for-each (lambda (i) (fun (car i) (cdr i))) node)))
(define (for-each-object-item-path root path fun) (for-each-object-item (search-desc root path) fun))
(define (for-each-verb fun) (for-each-object-item-path in-desc '("verbs") fun)) 
(define (for-each-event fun) (for-each-object-item-path in-desc '("events") fun)) 
(define (for-each-state-machine fun) (for-each-object-item-path in-desc '("state-machines") fun)) 

(for-each-verb (lambda (name desc)
   (print "VERB: " name "\n")))
(newline)

(for-each-event (lambda (name desc) (for-each display (list "EVENT: " name "\n"))))
(newline)
(for-each-state-machine (lambda (name desc) (for-each display (list "SM: " name "\n"))))
(newline)
; print the predeclaration of the verbs
(for-each-verb (lambda (name desc)
   (print (var 'scope) "void " (var 'prefix) (C-ify name) (var 'postfix) "(afb_req req);\n")))
(newline)

; print the array of verbs
(print
   "\n"
   "static const struct afb_verb_v3 _afb_verbs_" (C-ify (var 'api)) "[] =\n"
   "{\n")
(for-each-verb (lambda (name desc) (print
   "   {\n"
   "      .verb = " (C-string name) ",\n"
   "      .callback = " (var 'prefix) (C-ify name) (var 'postfix) ",\n"
   "      .info = " (C-string-null (search-desc desc '(description))) ",\n"
   "      .auth = NULL,\n"
   "      .vcbdata = NULL,\n"
   "      .session = 0,\n"
   "      .glob = 0\n"
   "   },\n")))
(print
   "   { .callback = NULL } /* END OF THE VERB ARRAY */\n"
   "};\n"
   "\n")

; perms-list handles the permissions as an array
(define perms-list '())
; add the item in the therms-list and returns its index
(define (permlist-put! . item)
   (letrec ((plput! (lambda (lst itm idx)
      (if (equal? (car lst) itm)
    idx
    (if (null? (cdr lst))
       (begin
          (set-cdr! lst (list itm))
          (+ idx 1))
       (plput! (cdr lst) itm (+ idx 1)))))))
      (if (null? perms-list)
    (begin
       (set! perms-list (list item))
       0)
    (plput! perms-list item 0))))

(define (declare-perms root session?)
   (letrec (
      (add (lambda (lst num)
    (if (null? lst)
       (list num)
       (if (< num (car lst))
          (cons num lst)
          (cons (car lst) (add (cdr lst) num))))))
      (putvec (lambda (vec sess?)
    (let ((l '()))
       (for-each
          (lambda (node)
        (let ((x (declare-perms node sess?)))
           (if x (set! l (add l x)))))
          (vector->list vec))
       l)))
      (putv (lambda (tag vec sess?)
    (do ((l (putvec vec sess?) (cdr l))
         (r #f (if r (permlist-put! tag (car l) r) (car l))))
        ((null? l) r)))))
   (cond
      ((equal? (car root) "anyOf")      (putv 'or (cdr root) #t))
      ((equal? (car root) "allOf")      (putv 'and (cdr root) #t))
      ((equal? (car root) "not")   (permlist-put! 'not (declare-perms (cdr root) #t)))
      ((equal? (car root) "permission") (permlist-put! 'perm (cdr root)))
      ((equal? (car root) "LOA")   (and session? (permlist-put! 'LOA (cdr root))))
      ((equal? (car root) "yes")   (and session? (permlist-put! 'yes)))
      ((equal? (car root) "no")    (and session? (permlist-put! 'no)))
      ((equal? (car root) "session")    (and session? (permlist-put! 'ses)))
      (else   #f))))

