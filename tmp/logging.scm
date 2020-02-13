; logging
(define loglevel 2)
(define (loglevel? x) (>= loglevel x))
(define-syntax iflog
   (syntax-rules ()
      ((_ level . what)
         (when (loglevel? level) . what))))

; printing
(define (print . args)
   (for-each display args))
(define-syntax dolog
   (syntax-rules ()
      ((_ level . what)
         (iflog level (print . what)))))

