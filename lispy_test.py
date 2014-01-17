from lispy import *
import unittest

class lis_tests(unittest.TestCase):
	def setUp(self):
		self.tests = [
			("(begin (define x 1) (set! x (+ x 1)) (+ x 1))", 3),
			("((lambda (x) (+ x x)) 5)", 10),
			("(define twice (lambda (x) (* 2 x)))", None),
			("(twice 5)", 10),
			("(define compose (lambda (f g) (lambda (x) (f (g x)))))", None),
			("((compose list twice) 5)", [10]),
			("(define repeat (lambda (f) (compose f f)))", None),
			("((repeat twice) 5)", 20),
			("((repeat (repeat twice)) 5)", 80),
			("(define fact (lambda (n) (if (<= n 1) 1 (* n (fact (- n 1))))))", None),
			("(fact 3)", 6),
			("(fact 50)", 30414093201713378043612608166064768844377641568960512000000000000),
			("""(define combine (lambda (f)
				(lambda (x y)
					(if (null? x) (quote ())
						(f (list (car x) (car y))
						((combine f) (cdr x) (cdr y)))))))""", None),
			("(define zip (combine cons))", None),
			("(zip (list 1 2 3 4) (list 5 6 7 8))", [[1, 5], [2, 6], [3, 7], [4, 8]]),
			("""(define riff-shuffle (lambda (deck) (begin
				(define take (lambda (n seq) (if (<= n 0) (quote ()) (cons (car seq) (take (- n 1) (cdr seq))))))
				(define drop (lambda (n seq) (if (<= n 0) seq (drop (- n 1) (cdr seq)))))
				(define mid (lambda (seq) (/ (length seq) 2)))
				((combine append) (take (mid deck) deck) (drop (mid deck) deck)))))""", None),
			("(riff-shuffle (list 1 2 3 4 5 6 7 8))", [1, 5, 2, 6, 3, 7, 4, 8]),
			("((repeat riff-shuffle) (list 1 2 3 4 5 6 7 8))",  [1, 3, 5, 7, 2, 4, 6, 8]),
			("(riff-shuffle (riff-shuffle (riff-shuffle (list 1 2 3 4 5 6 7 8))))", [1,2,3,4,5,6,7,8]),
		]

		self.lispy_tests = [
			("(define lyst (lambda items items))", None),
			("(lyst 1 2 3 (+ 2 2))", [1,2,3,4]),
			#("""(define (sum-squares-range start end)
			#	(define (sumsq-acc start end acc)
			#		(if (> start end) acc (sumsq-acc (+ start 1) end (+ (* start start) acc))))
			#	(sumsq-acc start end 0))""", None),
			#("(sum-squares-range 1 3000)", 9004500500), ## Tests tail recursion
			#("(call/cc (lambda (throw) (+ 5 (* 10 (throw 1))))) ;; throw", 1),
			#("(call/cc (lambda (throw) (+ 5 (* 10 1)))) ;; do not throw", 15),
			#("""(call/cc (lambda (throw) 
			#	(+ 5 (* 10 (call/cc (lambda (escape) (* 100 (escape 3)))))))) ; 1 level""", 35),
			#("""(call/cc (lambda (throw) 
			#	(+ 5 (* 10 (call/cc (lambda (escape) (* 100 (throw 3)))))))) ; 2 levels""", 3),
			#("""(call/cc (lambda (throw) 
			#	(+ 5 (* 10 (call/cc (lambda (escape) (* 100 1))))))) ; 0 levels""", 1005),
			#("(* 1i 1i)", -1),
			#("(sqrt -1)", 1j),
			#("(let ((a 1) (b 2)) (+ a b))", 3),
			#("(and 1 2 3)", 3),
			#("(and (> 2 1) 2 3)", 3),
			#("(and)", True),
			#("(and (> 2 1) (> 2 3))", False),
			#("(define-macro unless (lambda args `(if (not ,(car args)) (begin ,@(cdr args))))) ; test `", None),
			#("(unless (= 2 (+ 1 1)) (display 2) 3 4)", None),
			#(r'(unless (= 4 (+ 1 1)) (display 2) (display "\n") 3 4)', 4),
			#("(quote x)", 'x'), 
			#("(quote (1 2 three))", [1, 2, 'three']), 
			#("'x", 'x'),
			#("'(one 2 3)", ['one', 2, 3]),
			#("(define L (list 1 2 3))", None),
			#("`(testing ,@L testing)", ['testing',1,2,3,'testing']),
			#("`(testing ,L testing)", ['testing',[1,2,3],'testing']),
			#("""'(1 ;test comments '
			#	;skip this line
			#	2 ; more ; comments ; ) )
			#	3) ; final comment""", [1,2,3]),
		]

	def test_account(self):
		account_test = [
			("(define ((account bal) amt) (set! bal (+ bal amt)) bal)", None),
			("(define a1 (account 100))", None),
			("(a1 0)", 100),
			("(a1 10)", 110),
			("(a1 10)", 120),
		]
		runtime = Runtime()
		for (expression, expected) in account_test:
			self.assertEqual(runtime.eval(runtime.parse(expression)), expected)
	
	def test_newton(self):
		newton_tests = [
			("(define abs (lambda (n) ((if (> n 0) + -) 0 n)))", None),
			("(list (abs -3) (abs 0) (abs 3))", [3, 0, 3]),
			("""(define (newton guess function derivative epsilon)
				(define guess2 (- guess (/ (function guess) (derivative guess))))
				(if (< (abs (- guess guess2)) epsilon) guess2
					(newton guess2 function derivative epsilon)))""", None),
			("""(define (square-root a)
				(newton 1 (lambda (x) (- (* x x) a)) (lambda (x) (* 2 x)) 1e-8))""", None),
			("(> (square-root 200.) 14.14213)", True),
			("(< (square-root 200.) 14.14215)", True),
			("(= (square-root 200.) (sqrt 200.))", True),
		]
		runtime = Runtime()
		for (expression, expected) in newton_tests:
			self.assertEqual(runtime.eval(runtime.parse(expression)), expected)

	def test_syntax_errors(self):
		runtime = Runtime()
		syntax_errors = [
			"()",
			"(set! x)",
			"(define 3 4)",
			"(quote 1 2)",
			"(if 1 2 3 4)", 
			"(lambda 3 3)",
			"(lambda (x))",
			"""(if (= 1 2) (define-macro a 'a) 
			   (define-macro a 'b))""",
			"`,@L",
			#"(let ((a 1) (b 2 3)) (+ a b))",
		]
		for expression in syntax_errors:
			with self.assertRaises(SyntaxError):
				runtime.eval(runtime.parse(expression))
	
	def test_lookup_errors(self):
		runtime = Runtime()
		lookup_errors = [
			"(foo 1 2)",
			"x"
		]
		for expression in lookup_errors:
			with self.assertRaises(LookupError):
				runtime.eval(runtime.parse(expression))

	def test_type_errors(self):
		runtime = Runtime()
		type_errors = [
			#"(twice 2 2)",
		]
		for expression in type_errors:
			with self.assertRaises(TypeError):
				runtime.eval(runtime.parse(expression))


	def test_quote(self):
		quote_tests = [
			("(quote (testing 1 (2.0) -3.14e159))", ['testing', 1, [2.0], -3.14e159]),
		]
		runtime = Runtime()
		for (expression, expected) in quote_tests:
			self.assertEqual(runtime.eval(runtime.parse(expression)), expected)

	def test_twice(self):
		twice_tests = [
			("(define (twice x) (* 2 x))", None),
			("(twice 2)", 4),
		]
		runtime = Runtime()
		for (expression, expected) in twice_tests:
			self.assertEqual(runtime.eval(runtime.parse(expression)), expected)

	def test_basic_math(self):
		basic_math_tests = [
			("(+ 2 2)", 4),
			("(+ (* 2 100) (* 1 10))", 210),
		]
		runtime = Runtime()
		for (expression, expected) in basic_math_tests:
			self.assertEqual(runtime.eval(runtime.parse(expression)), expected)

	def test_if(self):
		if_tests = [
			("(if 1 2)", 2),
			("(if (= 3 4) 2)", None),
			("(if (> 6 5) (+ 1 1) (+ 2 2))", 2),
			("(if (< 6 5) (+ 1 1) (+ 2 2))", 4),
		]
		runtime = Runtime()
		for (expression, expected) in if_tests:
			self.assertEqual(runtime.eval(runtime.parse(expression)), expected)

	def test_environement(self):
		env_tests = [
			("(define x 3)", None),
			("x", 3),
			("(+ x x)", 6),
		]
		runtime = Runtime()
		for (expression, expected) in env_tests:
			self.assertEqual(runtime.eval(runtime.parse(expression)), expected)

	def test_advanced(self):
		runtime = Runtime()
		for (expression, expected) in self.lispy_tests:
			parsed = runtime.parse(expression)
			result = runtime.eval(parsed)
			self.assertEqual(result, expected)


if __name__ == '__main__':
	unittest.main()

