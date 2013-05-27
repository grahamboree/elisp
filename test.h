#pragma once
// Unit tests for elisp

/*TEST(tokenization, simple_add) {
	list<string> tokens = tokenize("(+ 1 2) ");
	list<string>::iterator tokenIter = tokens.begin();
	list<string>::iterator tokensEnd = tokens.end();

	for (;tokenIter != tokensEnd; ++tokenIter) {
		//cout << "\"" << *tokenIter << "\"" << endl;
	}
}

TEST(parsingAtoms, positive_int) {
	string code = "1";
	//cout << "atom(\"" << code << "\")" << endl;
	cell_t* a = atom(code);
	//cout << to_string(a) << endl;
	//cout << "======================" << endl;
}

TEST(parsingAtoms, negative_int) {
	string code = "-1";
	//cout << "atom(\"" << code << "\")" << endl;
	cell_t* a = atom(code);
	//cout << to_string(a) << endl;
	//cout << "======================" << endl;
}

TEST(parsingAtoms, negative_float) {
	string code = "-1.4";
	//cout << "atom(\"" << code << "\")" << endl;
	cell_t* a = atom(code);
	//cout << to_string(a) << endl;
	//cout << "======================" << endl;
}

TEST(parsingAtoms, positive_float) {
	string code = "1.4";
	//cout << "atom(\"" << code << "\")" << endl;
	cell_t* a = atom(code);
	//cout << to_string(a) << endl;
	//cout << "======================" << endl;
}

TEST(parsing, readAndToString) {
	string code = "(+ 1 2)";
	//cout << "read(\"" << code << "\")" << endl;
	cell_t* a = read(code);
	//cout << to_string(a) << endl;
	//cout << "======================" << endl;
	
	code = "(+ (- -1 3) 2)";
	//cout << "read(\"" << code << "\")" << endl;
	a = read(code);
	//cout << to_string(a) << endl;
}

TEST(ifelsefunc, nested) {
	string code = "(if #f 1 (if #T 4 2))";
	//cout << "eval(read(\"" << code << "\"))" << endl;
	//EXPECT_EQ(0, strcmp("4", to_string(eval(read(code))).c_str()));
	EXPECT_EQ("4", to_string(eval(read(code))));
}*/

class NorvigLispyTests : public testing::Test {
protected:
	void testCase(string inCode, string inExpected, Env* inEnv = global_env) {
		EXPECT_EQ(inExpected, to_string(eval(read(inCode), inEnv)));
	}
};

// Norvigs test cases
TEST_F(NorvigLispyTests, quote) {
	//testCase("(quote (testing 1 (2.0) -3.14e159))", "(testing 1 (2.0) -3.14e159)");
}

TEST_F(NorvigLispyTests, add) {
	testCase("(+ 2 2)", "4");
}

TEST_F(NorvigLispyTests, add_mul) {
    testCase("(+ (* 2 100) (* 1 10))", "210");
}

TEST_F(NorvigLispyTests, greater) {
	testCase("(if (> 6 5) (+ 1 1) (+ 2 2))", "2");
}

TEST_F(NorvigLispyTests, less) {
	testCase("(if (< 6 5) (+ 1 1) (+ 2 2))", "4");
}

TEST_F(NorvigLispyTests, define_var) {
	Env* testEnv = new Env(global_env);
	//testCase("(define x 3)", "", testEnv);
	testCase("(define x 3)", "'()", testEnv);
	testCase("x", "3", testEnv);
	testCase("(+ x x)", "6", testEnv);
}


/*
    (, None), ("x", 3), ("(+ x x)", 6),
    ("(begin (define x 1) (set! x (+ x 1)) (+ x 1))", 3),
    ("((lambda (x) (+ x x)) 5)", 10),
    ("(define twice (lambda (x) (* 2 x)))", None), ("(twice 5)", 10),
    ("(define compose (lambda (f g) (lambda (x) (f (g x)))))", None),
    ("((compose list twice) 5)", [10]),
    ("(define repeat (lambda (f) (compose f f)))", None),
    ("((repeat twice) 5)", 20), ("((repeat (repeat twice)) 5)", 80),
    ("(define fact (lambda (n) (if (<= n 1) 1 (* n (fact (- n 1))))))", None),
    ("(fact 3)", 6),
    ("(fact 50)", 30414093201713378043612608166064768844377641568960512000000000000),
    ("(define abs (lambda (n) ((if (> n 0) + -) 0 n)))", None),
    ("(list (abs -3) (abs 0) (abs 3))", [3, 0, 3]),
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
*/






/*
    ("()", SyntaxError), ("(set! x)", SyntaxError), 
    ("(define 3 4)", SyntaxError),
    ("(quote 1 2)", SyntaxError), ("(if 1 2 3 4)", SyntaxError), 
    ("(lambda 3 3)", SyntaxError), ("(lambda (x))", SyntaxError),
    ("""(if (= 1 2) (define-macro a 'a) 
     (define-macro a 'b))""", SyntaxError),
    ("(define (twice x) (* 2 x))", None), ("(twice 2)", 4),
    ("(twice 2 2)", TypeError),
    ("(define lyst (lambda items items))", None),
    ("(lyst 1 2 3 (+ 2 2))", [1,2,3,4]),
    ("(if 1 2)", 2),
    ("(if (= 3 4) 2)", None),
    ("(define ((account bal) amt) (set! bal (+ bal amt)) bal)", None),
    ("(define a1 (account 100))", None),
    ("(a1 0)", 100), ("(a1 10)", 110), ("(a1 10)", 120),
    ("""(define (newton guess function derivative epsilon)
    (define guess2 (- guess (/ (function guess) (derivative guess))))
    (if (< (abs (- guess guess2)) epsilon) guess2
        (newton guess2 function derivative epsilon)))""", None),
    ("""(define (square-root a)
    (newton 1 (lambda (x) (- (* x x) a)) (lambda (x) (* 2 x)) 1e-8))""", None),
    ("(> (square-root 200.) 14.14213)", True),
    ("(< (square-root 200.) 14.14215)", True),
    ("(= (square-root 200.) (sqrt 200.))", True),
    ("""(define (sum-squares-range start end)
         (define (sumsq-acc start end acc)
            (if (> start end) acc (sumsq-acc (+ start 1) end (+ (* start start) acc))))
         (sumsq-acc start end 0))""", None),
    ("(sum-squares-range 1 3000)", 9004500500), ## Tests tail recursion
    ("(call/cc (lambda (throw) (+ 5 (* 10 (throw 1))))) ;; throw", 1),
    ("(call/cc (lambda (throw) (+ 5 (* 10 1)))) ;; do not throw", 15),
    ("""(call/cc (lambda (throw) 
         (+ 5 (* 10 (call/cc (lambda (escape) (* 100 (escape 3)))))))) ; 1 level""", 35),
    ("""(call/cc (lambda (throw) 
         (+ 5 (* 10 (call/cc (lambda (escape) (* 100 (throw 3)))))))) ; 2 levels""", 3),
    ("""(call/cc (lambda (throw) 
         (+ 5 (* 10 (call/cc (lambda (escape) (* 100 1))))))) ; 0 levels""", 1005),
    ("(* 1i 1i)", -1), ("(sqrt -1)", 1j),
    ("(let ((a 1) (b 2)) (+ a b))", 3),
    ("(let ((a 1) (b 2 3)) (+ a b))", SyntaxError),
    ("(and 1 2 3)", 3), ("(and (> 2 1) 2 3)", 3), ("(and)", True),
    ("(and (> 2 1) (> 2 3))", False),
    ("(define-macro unless (lambda args `(if (not ,(car args)) (begin ,@(cdr args))))) ; test `", None),
    ("(unless (= 2 (+ 1 1)) (display 2) 3 4)", None),
    (r'(unless (= 4 (+ 1 1)) (display 2) (display "\n") 3 4)', 4),
    ("(quote x)", 'x'), 
    ("(quote (1 2 three))", [1, 2, 'three']), 
    ("'x", 'x'),
    ("'(one 2 3)", ['one', 2, 3]),
    ("(define L (list 1 2 3))", None),
    ("`(testing ,@L testing)", ['testing',1,2,3,'testing']),
    ("`(testing ,L testing)", ['testing',[1,2,3],'testing']),
    ("`,@L", SyntaxError),
    ("""'(1 ;test comments '
     ;skip this line
     2 ; more ; comments ; ) )
     3) ; final comment""", [1,2,3]),
    ]
 */
