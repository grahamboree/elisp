
; test comment
(display (quote 4))

; returns the incremented 
(define (inc n) (+ n 1))
(display (inc 4))

; returns the nth fibbonacci number
(define (fib n) (if (= n 1) 1 (if (= n 2) 1 (+ (fib (- n 1)) (fib (- n 2)))))) 

(display "this should be 55: " (fib 10))

(define (x f) (lambda (g) (+ f g)))
(display "this should be 7: " ((x 4) 3))
