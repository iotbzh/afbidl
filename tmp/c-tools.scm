; compute the C presentation of the string of 'value
; prefix is the string to prepend at the beginning
; width if the expected width in characters (tab is 1 char)
(define (C-string-fmt value prefix width)
        (let* (
                (esc (lambda (c fun) ; emit escaped c to fun
                                (case c
                                        ((#\" #\\) (fun #\\) (fun c))
                                        ((#\newline) (fun #\\) (fun #\n))
                                        ((#\x0D) (fun #\\) (fun #\r))
                                        ((#\tab) (fun #\\) (fun #\t))
                                        (else (fun c)))))
                (process (lambda (str fun) ; emit c block for str to fun
                                (let* (
                                        (w 0)
                                        (start (lambda (fun) (set! w 0) (string-for-each fun prefix) (fun #\")))
                                        (stop  (lambda (fun) (fun #\")))
                                        (start-if (lambda (fun limit)
                                        	(when (>= w limit) (stop fun) (fun #\newline) (start fun))))
                                        (emit  (lambda (c fun) (fun c) (set! w (+ w 1))))
                                        (width-emit  (lambda (c fun) (start-if fun width) (esc c (lambda (c) (emit c fun)))))
                                )
                                        (start fun)
                                        (string-for-each (lambda (c) (width-emit c fun)) str)
                                        (stop fun))))
                (count (lambda (str) ; count the number of characters of the c block of str
                                (let ((r 0)) (process str (lambda (c) (set! r (+ r 1)))) r)))
                (result (make-string (count value))) ; create a string of the block size
                (fill (lambda (str) ; fill the result string with c block of str
                                (let ((r 0)) (process str (lambda (c) (string-set! result r c) (set! r (+ r 1)))))))
                )
                (fill value)
                result))

; default C formating for inline values
(define (C-string value) (C-string-fmt value "" 256))

; default formating or NULL
(define (C-string-null value) (if (string? value) (C-string value) "NULL"))

; return a C compatible identifier for name
(define (C-ify name)
        (string-map (lambda (c)
                (if (or (char-alphabetic? c) (char-numeric? c)) c #\_))
                name))
