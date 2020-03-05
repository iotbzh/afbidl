
const ctools = require('./c-tools.js');
const otools = require('./o-tools.js');
const fs = require('fs');

// get the arguments

var inFile = process.argv[2];
console.log("processing file %s", inFile);

// open the input JSON file
console.log("reading file %s", inFile);
var inContent = fs.readFileSync(inFile);
console.log("content of file %s is [[[%s]]]", inFile, inContent);

// process the json input
console.log("parsing content of $%s...", inFile);
var inDesc = JSON.parse(inContent);

// show the input as pretty JSON
console.log("got for ${inFile}: %s", JSON.stringify(inDesc,null,3));

// compute the C presentation
var outCColumn = 120;
var outCTabs = "   ";
var cDesc = ctools.stringFmt(JSON.stringify(inDesc), outCTabs, outCColumn - 1 - outCTabs.length);
console.log("inDescC:\n%s", cDesc);

// make the Doc
var doc = new otools.ODoc(inDesc);

// initialize the variable
var vars = {};
[
   [ "preinit",      "#/tools/afb-genskel/preinit" ],
   [ "init",         "#/tools/afb-genskel/init" ],
   [ "onevent",      "#/tools/afb-genskel/onevent" ],
   [ "scope",        "#/tools/afb-genskel/scope",        "static" ],
   [ "prefix",       "#/tools/afb-genskel/prefix",       "afb_verb_" ],
   [ "postfix",      "#/tools/afb-genskel/postfix",      "_cb" ],
   [ "provideclass", "#/tools/afb-genskel/provide-class" ],
   [ "requireclass", "#/tools/afb-genskel/require-class" ],
   [ "requireapi",   "#/tools/afb-genskel/require-api" ],
   [ "priv",         "#/tools/afb-genskel/private",       false ],
   [ "noconc",       "#/tools/afb-genskel/noconcurrency", false ],
   [ "api",          "#/info/apiname" ],
   [ "title",        "#/info/title",                      "?" ],
   [ "desc",         "#/info/description" ]

].forEach(function(item){
   var v = doc.nodeAt(item[1]);
   if (v === undefined && item[2] !== undefined)
      v = item[2];
   vars[item[0]] = v;
});


function forEachOfDoc(path, fun) {
   var r = doc.at(path);
   if (r) {
      r.expand(false);
      for (let i in r.children)
         fun(r.children[i]);
    }
}
function forEachVerb(fun) { forEachOfDoc(["verbs"], fun); }
function forEachEvent(fun) { forEachOfDoc(["events"], fun); }
function forEachStateMachine(fun) { forEachOfDoc(["state-machines"], fun); }





forEachVerb(function(n) {
   console.log(vars['scope'] + " void " + vars['prefix'] + ctools.id(n.name) +  vars['postfix'] + "(afb_req req);\n");
});









/*
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
   "   { .verb = NULL, .callback = NULL } /* END OF THE VERB ARRAY * /\n"
   "};\n"
   "\n")

; perms-list handles the permissions as an array
(define perms-list '())

; add the item in the perms-list and returns its index
(define (permlist-put! . item)
   (letrec ((plput! (
      lambda (lst itm idx)
         (cond
            ((equal? (car lst) itm) idx)
            ((null? (cdr lst)) (begin (set-cdr! lst (list itm)) (+ idx 1)))
            (else  (plput! (cdr lst) itm (+ idx 1))))))))
      (if (null? perms-list)
         (begin (set! perms-list (list item)) 0)
         (plput! perms-list item 0)))

(define (declare-perms root session?)
   (letrec (
      (add (lambda (lst num) ; add num and return the sorted list
         (cond
            ((null? lst)       (list num))
            ((= num (car lst)) lst)
            ((< num (car lst)) (cons num lst))
            (else              (cons (car lst) (add (cdr lst) num))))))
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
         ((equal? (car root) "not")        (permlist-put! 'not (declare-perms (cdr root) #t)))
         ((equal? (car root) "permission") (permlist-put! 'perm (cdr root)))
         ((equal? (car root) "LOA")        (and session? (permlist-put! 'LOA (cdr root))))
         ((equal? (car root) "yes")        (and session? (permlist-put! 'yes)))
         ((equal? (car root) "no")         (and session? (permlist-put! 'no)))
         ((equal? (car root) "session")    (and session? (permlist-put! 'ses)))
         (else   #f))))

; print the predeclaration of the verbs
(for-each-verb (lambda (name desc)
   (print (var 'scope) "void " (var 'prefix) (C-ify name) (var 'postfix) "(afb_req req);\n")))
(newline)
*/