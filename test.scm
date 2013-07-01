
; test comment
(display (quote 4))

; returns the incremented 
(define (inc n) (+ n 1))
(display (inc 4))

; returns the nth fibbonacci number
(define (fib n)
  (if (= n 1) 1
	(if (= n 2) 1
	  (+ (fib (- n 1)) (fib (- n 2))))))

(display (fib 10))


;(define deriv
;  (lambda (f)
;	(lambda (x)
;	  (/ (- (f (+ x dx))
;			(f x))
;		 dx))))

;(define (deriv exp var) 
;  (cond ((constant? exp var) 0)
;		((same-var? exp var) 1)
;		((sum? exp) (make-sum (deriv (a1 exp) var) (deriv (a2 exp) var)))
;		((product? exp) 
;		 (make-product (m1 exp) (deriv (m2 exp) var))
;		 (make-product (deriv (m1 exp) var) (m2 exp)))))
;
;(define (constant? exp var) (and (atom? exp) (not (eq? exp var))))
;(define (same-var? exp var) (and (atom? exp) (eq? exp var)))
;(define (sum? exp)
;  (and (not (atom? exp))
;	   (eq? (car exp) '+)))
;(define (make-sum a1 a2) (list '+ a1 a2))
;(define a1 cadr)
;(define a2 caddr)
;(define (product? exp) (and (not (atom? exp)) (eq? (car exp) '*)))
;(define (mkae-product m1 m2) (list '* m1 m2))
;(define m1 cadr)
;(define m2 caddr)

