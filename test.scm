
(define (cons x y)
  (lambda (m) (m x y)))

(define (car x)
  (x (lambda (a d) a)))

(define (cdr x)
  (x (lambda (a d) d)))

(define else #t)

(define (cond x)
	(if (car (car x)) (cdr (car x))
	  (cond (cdr x))))

(define (+1 n) (+ n 1))

; test comment
(display (quote 4))
(display (+1 4))

(display (cond (cons (cons #f 1)
					 (cons (cons #f 2)
						   (cons (cons #f 3)
								 (cons (cons else 4) (quote ())))))))

; returns the nth fibbonacci number
(define (fib n) (if (= n 1) 1 (if (= n 2) 1 (+ (fib (- n 1)) (fib (- n 2)))))) 

(display "this should be 55: " (fib 10))

(define (x f) (lambda (g) (+ f g)))
(display "this should be 7: " ((x 4) 3))
