//
//  test.h
//  Unit Tests for elisp
//
//  Created by Graham Pentheny on 12/21/14.
//  Copyright (c) 2014 Graham Pentheny. All rights reserved.
//

#pragma once

namespace elisp {

string result(string inCode, Program& program) {
	return program.runCode(inCode);
}

string result(string inCode) {
	Program testProgram;
	return testProgram.runCode(inCode); 
}

// Norvigs test cases
TEST_CASE("NorvigLispyTests/quote", "quote") {
	REQUIRE(result("(quote (testing 1 (2.0) -3.14e159))") == "(testing 1 (2.0) -3.14e159)");
}

TEST_CASE("NorvigLispyTests/add", "add") {
	REQUIRE(result("(+ 2 2)") == "4");
}

TEST_CASE("NorvigLispyTests/add_mul", "add_mul") {
    REQUIRE(result("(+ (* 2 100) (* 1 10))") == "210");
}

TEST_CASE("NorvigLispyTests/greater", "greater") {
	REQUIRE(result("(if (> 6 5) (+ 1 1) (+ 2 2))") == "2");
}

TEST_CASE("NorvigLispyTests/less", "less") {
	REQUIRE(result("(if (< 6 5) (+ 1 1) (+ 2 2))") == "4");
}

TEST_CASE("NorvigLispyTests/define_var", "define_var") {
	Program testProgram;
	REQUIRE(result("(define x 3)", testProgram) == "'()");
	REQUIRE(result("x", testProgram) == "3");
	REQUIRE(result("(+ x x)", testProgram) == "6");
}

TEST_CASE("closures", "") {
	Program testProgram;
	testProgram.runCode("(define (x f) (lambda (g) (+ f g)))");
	REQUIRE(result("((x 4) 3)", testProgram) == "7");
}
/*
    ("(quote (testing 1 (2.0) -3.14e159))", ['testing', 1, [2.0], -3.14e159]),
    ("(+ 2 2)", 4),
    ("(+ (* 2 100) (* 1 10))", 210),
    ("(if (> 6 5) (+ 1 1) (+ 2 2))", 2),
    ("(if (< 6 5) (+ 1 1) (+ 2 2))", 4),
    ("(define x 3)", None),
	("x", 3), ("(+ x x)", 6),
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
    ]

lispy_tests = [
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

    
    TEST_CASE("prelude/add", "+") {
        Env testEnv = std::make_shared<Environment>();
        add_globals(testEnv);
        Cell result;
        number_cell* number;
        auto oneVal = std::make_shared<number_cell>(1.0);
        
        // One argument
        auto oneArg = makeList({oneVal});
        
        result = add(oneArg, testEnv);
        REQUIRE(result->GetType() == kCellType_number);
        
        number = static_cast<number_cell*>(result.get());
        REQUIRE(number->GetValue() == 1);
        
        // Two arguments
        auto twoArgs = makeList({oneVal, oneVal});
        
        result = add(twoArgs, testEnv);
        REQUIRE(result->GetType() == kCellType_number);
        
        number = static_cast<number_cell*>(result.get());
        REQUIRE(number->GetValue() == 2);
        
        // Seven arguments
        shared_ptr<cons_cell> sevenArgs = makeList({
            oneVal,
            oneVal,
            oneVal,
            oneVal,
            oneVal,
            oneVal,
            oneVal});
        
        result = add(sevenArgs, testEnv);
        REQUIRE(result->GetType() == kCellType_number);
        
        number = static_cast<number_cell*>(result.get());
        REQUIRE(number->GetValue() == 7);
        
        // Nested
        shared_ptr<cons_cell> nested = makeList({
            oneVal,
            makeList({
                std::make_shared<symbol_cell>("+"),
                oneVal,
                oneVal})
        });
        
        result = add(nested, testEnv);
        REQUIRE(result->GetType() == kCellType_number);
        
        number = static_cast<number_cell*>(result.get());
        REQUIRE(number->GetValue() == 3);
    }
    TEST_CASE("prelude/sub", "-") {
        Env testEnv = std::make_shared<Environment>();
        add_globals(testEnv);
        auto oneVal = std::make_shared<number_cell>(1.0);
        
        // One argument
        auto result = sub(makeList({oneVal}), testEnv);
        REQUIRE(result->GetType() == kCellType_number);
        
        auto number = std::static_pointer_cast<number_cell>(result);
        REQUIRE(number->GetValue() == -1);
        
        // Two arguments
        result = sub(makeList({oneVal, oneVal}), testEnv);
        REQUIRE(result->GetType() == kCellType_number);
        
        number = std::static_pointer_cast<number_cell>(result);
        REQUIRE(number->GetValue() == 0);
        
        // Seven arguments
        result = sub(makeList({
            std::make_shared<number_cell>(10.0),
            oneVal,
            oneVal,
            oneVal,
            oneVal,
            oneVal,
            oneVal}), testEnv);
        REQUIRE(result->GetType() == kCellType_number);
        
        number = std::static_pointer_cast<number_cell>(result);
        REQUIRE(number->GetValue() == 4);
        
        // Nested
        shared_ptr<cons_cell> nested = makeList({
            std::make_shared<number_cell>(5.0),
            makeList({
                std::make_shared<symbol_cell>("-"),
                std::make_shared<number_cell>(2.0),
                oneVal}),
            oneVal});
        
        result = sub(nested, testEnv);
        REQUIRE(result->GetType() == kCellType_number);
        
        number = std::static_pointer_cast<number_cell>(result);
        REQUIRE(number->GetValue() == 3);
    }
    TEST_CASE("prelude/mult", "*") {
        Env testEnv = std::make_shared<Environment>();
        add_globals(testEnv);
        
        // Two arguments
        auto result = mult(makeList({
            std::make_shared<number_cell>(1.0),
            std::make_shared<number_cell>(2.0)}), testEnv);
        REQUIRE(result->GetType() == kCellType_number);
        
        auto number = std::static_pointer_cast<number_cell>(result);
        REQUIRE(number->GetValue() == 2);
        
        // Seven arguments
        result = mult(makeList({
            std::make_shared<number_cell>(10.0),
            std::make_shared<number_cell>(2.0),
            std::make_shared<number_cell>(1.0),
            std::make_shared<number_cell>(-1.0),
            std::make_shared<number_cell>(0.5),
            std::make_shared<number_cell>(10.0),
            std::make_shared<number_cell>(-5.0)}), testEnv);
        REQUIRE(result->GetType() == kCellType_number);
        
        number = std::static_pointer_cast<number_cell>(result);
        REQUIRE(number->GetValue() == 500);
        
        // Nested
        result = mult(makeList({
            std::make_shared<number_cell>(2.0),
            makeList({
                std::make_shared<symbol_cell>("*"),
                std::make_shared<number_cell>(2.0),
                std::make_shared<number_cell>(3.0)}),
            std::make_shared<number_cell>(2.0)}), testEnv);
        REQUIRE(result->GetType() == kCellType_number);
        
        number = std::static_pointer_cast<number_cell>(result);
        REQUIRE(number->GetValue() == 24);
    }
    TEST_CASE("prelude/div", "/") {
        Env testEnv = std::make_shared<Environment>();
        add_globals(testEnv);
        
        // One argument
        auto result = div(makeList({std::make_shared<number_cell>(2.0)}), testEnv);
        REQUIRE(result->GetType() == kCellType_number);
        
        auto number = std::static_pointer_cast<number_cell>(result);
        REQUIRE(number->GetValue() == 0.5);
        
        // Two arguments
        result = div(makeList({
            std::make_shared<number_cell>(4.0),
            std::make_shared<number_cell>(2.0)}), testEnv);
        REQUIRE(result->GetType() == kCellType_number);
        
        number = std::static_pointer_cast<number_cell>(result);
        REQUIRE(number->GetValue() == 2);
        
        // Seven arguments
        result = div(makeList({
            std::make_shared<number_cell>(10.0),
            std::make_shared<number_cell>(2.0),
            std::make_shared<number_cell>(2.5)}), testEnv);
        REQUIRE(result->GetType() == kCellType_number);
        
        number = std::static_pointer_cast<number_cell>(result);
        REQUIRE(number->GetValue() == 2);
        
        // Nested
        result = div(makeList({
            std::make_shared<number_cell>(4.0),
            makeList({
                std::make_shared<symbol_cell>("/"),
                std::make_shared<number_cell>(2.0),
                std::make_shared<number_cell>(1.0)}),
            std::make_shared<number_cell>(2.0)}), testEnv);
        REQUIRE(result->GetType() == kCellType_number);
        
        number = std::static_pointer_cast<number_cell>(result);
        REQUIRE(number->GetValue() == 1);
    }
    TEST_CASE("prelude/eq", "=") {
        Env testEnv = std::make_shared<Environment>();
        add_globals(testEnv);
        
        // Two arguments
        auto result = eq(makeList({
            std::make_shared<number_cell>(2.0),
            std::make_shared<number_cell>(2.0)}), testEnv);
        REQUIRE(result->GetType() == kCellType_bool);
        REQUIRE(cell_to_bool(result));
        
        // Seven arguments
        result = eq(makeList({
            std::make_shared<number_cell>(-0.0),
            std::make_shared<number_cell>(0.0),
            std::make_shared<number_cell>(0.0)}), testEnv);
        REQUIRE(result->GetType() == kCellType_bool);
        REQUIRE(cell_to_bool(result));
        
        // Nested
        result = eq(makeList({
            std::make_shared<number_cell>(4.0),
            makeList({
                std::make_shared<symbol_cell>("*"),
                std::make_shared<number_cell>(2.0),
                std::make_shared<number_cell>(2.0)})}), testEnv);
        REQUIRE(result->GetType() == kCellType_bool);
        REQUIRE(cell_to_bool(result));
    }
    TEST_CASE("prelude/if", "if") {
        Env testEnv = std::make_shared<Environment>();
        add_globals(testEnv);
        
        // Equal
        auto result = if_then_else(makeList({
            makeList({
                std::make_shared<symbol_cell>("="),
                std::make_shared<number_cell>(1.0),
                std::make_shared<number_cell>(1.0)}),
            std::make_shared<number_cell>(1.0),
            std::make_shared<number_cell>(2.0)}), testEnv);
        REQUIRE(result->GetType() == kCellType_number);
        auto number = std::static_pointer_cast<number_cell>(result);
        REQUIRE(number->GetValue() == 1.0);
        
        // Not Equal
        result = if_then_else(makeList({
            makeList({
                std::make_shared<symbol_cell>("="),
                std::make_shared<number_cell>(2.0),
                std::make_shared<number_cell>(1.0)}),
            std::make_shared<number_cell>(1.0),
            std::make_shared<number_cell>(2.0)}), testEnv);
        REQUIRE(result->GetType() == kCellType_number);
        number = std::static_pointer_cast<number_cell>(result);
        REQUIRE(number->GetValue() == 2.0);
        
        // Constant
        result = if_then_else(makeList({
            std::make_shared<bool_cell>(false),
            std::make_shared<number_cell>(1.0),
            std::make_shared<number_cell>(2.0)}), testEnv);
        REQUIRE(result->GetType() == kCellType_number);
        number = std::static_pointer_cast<number_cell>(result);
        REQUIRE(number->GetValue() == 2.0);
        
        // Empty list
        result = if_then_else(makeList({
            std::make_shared<bool_cell>(empty_list),
            std::make_shared<number_cell>(1.0),
            std::make_shared<number_cell>(2.0)}), testEnv);
        REQUIRE(result->GetType() == kCellType_number);
        number = std::static_pointer_cast<number_cell>(result);
        REQUIRE(number->GetValue() == 2.0);
    }
    TEST_CASE("prelude/quote", "quote") {
        Env testEnv = std::make_shared<Environment>();
        add_globals(testEnv);
        
        Cell result = quote(makeList({makeList({
            std::make_shared<symbol_cell>("="),
            std::make_shared<number_cell>(1.0),
            std::make_shared<number_cell>(2.0)})}), testEnv);
        REQUIRE(result->GetType() == kCellType_cons);
        auto cons = std::static_pointer_cast<cons_cell>(result);
        auto it = cons->begin();
        
        REQUIRE((*it)->GetType() == kCellType_symbol);
        auto equalSymbol = std::static_pointer_cast<symbol_cell>(*it);
        REQUIRE(equalSymbol->GetIdentifier() == "=");
        
        ++it;
        REQUIRE((*it)->GetType() == kCellType_number);
        auto numberSymbol = std::static_pointer_cast<number_cell>(*it);
        REQUIRE(numberSymbol->GetValue() == 1.0);
        
        ++it;
        REQUIRE((*it)->GetType() == kCellType_number);
        numberSymbol = std::static_pointer_cast<number_cell>(*it);
        REQUIRE(numberSymbol->GetValue() == 2.0);
        
        ++it;
        REQUIRE(it == cons->end());
    }
    TEST_CASE("prelude/set!", "set!") {
        Env testEnv = std::make_shared<Environment>();
        add_globals(testEnv);
        testEnv->mSymbolMap["derp"] = std::make_shared<number_cell>(1.0);
        
        set(makeList({
            std::make_shared<symbol_cell>("derp"),
            std::make_shared<number_cell>(2.0)}), testEnv);
        REQUIRE(testEnv->find("derp") == testEnv);
        auto derpCell = testEnv->get("derp");
        REQUIRE(derpCell->GetType() == kCellType_number);
        auto num = std::static_pointer_cast<number_cell>(derpCell);
        REQUIRE(num->GetValue() == 2);
    }
    TEST_CASE("prelude/define", "define") {
        Env testEnv = std::make_shared<Environment>();
        add_globals(testEnv);
        
        define(makeList({
            std::make_shared<symbol_cell>("derp"),
            std::make_shared<number_cell>(2.0)}), testEnv);
        REQUIRE(testEnv->find("derp") == testEnv);
        auto derpCell = testEnv->get("derp");
        REQUIRE(derpCell->GetType() == kCellType_number);
        auto num = std::static_pointer_cast<number_cell>(derpCell);
        REQUIRE(num->GetValue() == 2);
    }
    TEST_CASE("prelude/cons", "cons") {
        Env testEnv = std::make_shared<Environment>();
        add_globals(testEnv);
        testEnv->mSymbolMap["derp"] = std::make_shared<number_cell>(1.0);
        
        Cell result = cons(makeList({std::make_shared<number_cell>(1.0), std::make_shared<number_cell>(2.0)}), testEnv);
        
        REQUIRE(result);
        REQUIRE(result->GetType() == kCellType_cons);
        
        auto consCell = std::static_pointer_cast<cons_cell>(result);
        
        REQUIRE(consCell->GetCar());
        REQUIRE(consCell->GetCar()->GetType() == kCellType_number);
        auto num = std::static_pointer_cast<number_cell>(consCell->GetCar());
        REQUIRE(num->GetValue() == 1.0);
        
        REQUIRE(consCell->GetCdr());
        REQUIRE(consCell->GetCdr()->GetType() == kCellType_number);
        num = std::static_pointer_cast<number_cell>(consCell->GetCdr());
        REQUIRE(num->GetValue() == 2.0);
    }
    TEST_CASE("prelude/car", "car") {
        Env testEnv = std::make_shared<Environment>();
        add_globals(testEnv);
        testEnv->mSymbolMap["derp"] = std::make_shared<number_cell>(1.0);
        
        auto expr = Program::read("(cons 1 2)");
        auto result = car(makeList({expr}), testEnv);
        
        REQUIRE(result);
        REQUIRE(result->GetType() == kCellType_number);
        auto num = std::static_pointer_cast<number_cell>(result);
        REQUIRE(num->GetValue() == 1.0);
    }
    TEST_CASE("prelude/cdr", "cdr") {
        Env testEnv = std::make_shared<Environment>();
        add_globals(testEnv);
        testEnv->mSymbolMap["derp"] = std::make_shared<number_cell>(1.0);
        
        auto expr = Program::read("(cons 1 2)");
        auto result = cdr(makeList({expr}), testEnv);
        
        REQUIRE(result);
        REQUIRE(result->GetType() == kCellType_number);
        auto num = std::static_pointer_cast<number_cell>(result);
        REQUIRE(num->GetValue() == 2.0);
    }
}
