// vim: set foldmethod=marker:
/*
 * The elisp standard prelude
 */

#pragma once

struct numerical_proc : public proc_cell {
protected:
	double getOpValue(cell_t* inOp) {
		trueOrDie((inOp->type == kCellType_number), "Expected only number arguments");
		return static_cast<number_cell*>(inOp)->value;
	}
};

struct add_proc : public numerical_proc { virtual cell_t* evalProc(list_cell* args, Environment& env); };
struct sub_proc : public numerical_proc { virtual cell_t* evalProc(list_cell* args, Environment& env); };
struct mult_proc: public numerical_proc { virtual cell_t* evalProc(list_cell* args, Environment& env); };
struct div_proc : public numerical_proc { virtual cell_t* evalProc(list_cell* args, Environment& env); };
struct eq_proc 	: public numerical_proc { virtual cell_t* evalProc(list_cell* args, Environment& env); };
struct if_proc 		: public proc_cell  { virtual cell_t* evalProc(list_cell* args, Environment& env); };
struct quote_proc 	: public proc_cell  { virtual cell_t* evalProc(list_cell* args, Environment&); };
struct set_proc 	: public proc_cell  { virtual cell_t* evalProc(list_cell* args, Environment& env); };
struct define_proc 	: public proc_cell  { virtual cell_t* evalProc(list_cell* args, Environment& env); };
struct lambda_proc 	: public proc_cell  { virtual cell_t* evalProc(list_cell* args, Environment&); };
struct begin_proc 	: public proc_cell  { virtual cell_t* evalProc(list_cell* args, Environment& env); };
struct let_proc 	: public proc_cell  { virtual cell_t* evalProc(list_cell* args, Environment& env); };
struct display_proc : public proc_cell  { virtual cell_t* evalProc(list_cell* args, Environment& env); };
struct exit_proc 	: public proc_cell  { virtual cell_t* evalProc(list_cell*, Environment&) { exit(0); } };
struct greater_proc : public proc_cell  { virtual cell_t* evalProc(list_cell* args, Environment& env); };
struct less_proc 	: public proc_cell  { virtual cell_t* evalProc(list_cell* args, Environment& env); };

void add_globals(Environment& env) {
	env.mSymbolMap["+"] 		= new add_proc;
	env.mSymbolMap["-"] 		= new sub_proc;
	env.mSymbolMap["*"] 		= new mult_proc;
	env.mSymbolMap["/"] 		= new div_proc;
	env.mSymbolMap["="] 		= new eq_proc;
	env.mSymbolMap[">"] 		= new greater_proc;
	env.mSymbolMap["<"] 		= new less_proc;
	env.mSymbolMap["if"] 		= new if_proc;
	env.mSymbolMap["begin"] 	= new begin_proc;
	env.mSymbolMap["define"] 	= new define_proc;
	env.mSymbolMap["lambda"] 	= new lambda_proc;
	env.mSymbolMap["quote"]		= new quote_proc;
	env.mSymbolMap["set!"] 		= new set_proc;
	env.mSymbolMap["let"] 		= new let_proc;
	env.mSymbolMap["display"] 	= new display_proc;
	env.mSymbolMap["exit"] 		= new exit_proc;
}

inline cell_t* add_proc::evalProc(list_cell* args, Environment& env) {
	verifyCell(args, "+");
	
	cons_cell* currentCell = args;
	double result = 0.0;
	while(currentCell) {
		result += getOpValue(env.eval(currentCell->car));
		currentCell = currentCell->cdr;
	}
	
	return new number_cell(result);
}

#ifdef ELISP_TEST // {{{
TEST_CASE("prelude/add", "+") {
	Environment testEnv;
	add_globals(testEnv);
	add_proc a;
	cell_t* result;
	number_cell* number;
	
	// One argument
	cons_cell* oneArg = makeList({new number_cell(1.0)});
	
	result = a.evalProc(oneArg, testEnv);
	REQUIRE(result->type == kCellType_number);
	
	number = static_cast<number_cell*>(result);
	REQUIRE(number->value == 1);
	
	// Two arguments
	cons_cell* twoArgs = makeList({
		new number_cell(1.0),
		new number_cell(1.0)});
	
	result = a.evalProc(twoArgs, testEnv);
	REQUIRE(result->type == kCellType_number);
	
	number = static_cast<number_cell*>(result);
	REQUIRE(number->value == 2);
	
	// Seven arguments
	cons_cell* sevenArgs = makeList({
		new number_cell(1.0),
		new number_cell(1.0),
		new number_cell(1.0),
		new number_cell(1.0),
		new number_cell(1.0),
		new number_cell(1.0),
		new number_cell(1.0)});
	
	result = a.evalProc(sevenArgs, testEnv);
	REQUIRE(result->type == kCellType_number);
	
	number = static_cast<number_cell*>(result);
	REQUIRE(number->value == 7);
	
	// Nested
	cons_cell* nested = makeList({
		new number_cell(1.0),
		makeList({
			new symbol_cell("+"),
			new number_cell(1.0),
			new number_cell(1.0)})});
	
	result = a.evalProc(nested, testEnv);
	REQUIRE(result->type == kCellType_number);
	
	number = static_cast<number_cell*>(result);
	REQUIRE(number->value == 3);
}
#endif // }}}

inline cell_t* sub_proc::evalProc(list_cell* args, Environment& env) {
	verifyCell(args, "-");

	cons_cell* currentCell = args;

	double result = getOpValue(env.eval(currentCell->car));
	currentCell = currentCell->cdr;

	if (!currentCell)
		return new number_cell(-result);

	while(currentCell) {
		result -= getOpValue(env.eval(currentCell->car));
		currentCell = currentCell->cdr;
	}

	return new number_cell(result); 
}

#ifdef ELISP_TEST // {{{
TEST_CASE("prelude/sub", "-") {
	Environment testEnv;
	add_globals(testEnv);
	sub_proc sub;
	cell_t* result;
	number_cell* number;
	
	// One argument
	cons_cell* oneArg = makeList({new number_cell(1.0)});
	
	result = sub.evalProc(oneArg, testEnv);
	REQUIRE(result->type == kCellType_number);
	
	number = static_cast<number_cell*>(result);
	REQUIRE(number->value == -1);
	
	// Two arguments
	cons_cell* twoArgs = makeList({
		new number_cell(1.0),
		new number_cell(1.0)});
	
	result = sub.evalProc(twoArgs, testEnv);
	REQUIRE(result->type == kCellType_number);
	
	number = static_cast<number_cell*>(result);
	REQUIRE(number->value == 0);
	
	// Seven arguments
	cons_cell* sevenArgs = makeList({
		new number_cell(10.0),
		new number_cell(1.0),
		new number_cell(1.0),
		new number_cell(1.0),
		new number_cell(1.0),
		new number_cell(1.0),
		new number_cell(1.0)});
	
	result = sub.evalProc(sevenArgs, testEnv);
	REQUIRE(result->type == kCellType_number);
	
	number = static_cast<number_cell*>(result);
	REQUIRE(number->value == 4);
	
	// Nested
	cons_cell* nested = makeList({
		new number_cell(5.0),
		makeList({
			new symbol_cell("-"),
			new number_cell(2.0),
			new number_cell(1.0)}),
		new number_cell(1.0)});
	
	result = sub.evalProc(nested, testEnv);
	REQUIRE(result->type == kCellType_number);
	
	number = static_cast<number_cell*>(result);
	REQUIRE(number->value == 3);
}
#endif // }}}

inline cell_t* mult_proc::evalProc(list_cell* args, Environment& env) {
	verifyCell(args, "*");

	double result = 1.0;
	cons_cell* currentCell = args;
	while(currentCell) {
		result *= getOpValue(env.eval(currentCell->car));
		currentCell = currentCell->cdr;
	}

	return new number_cell(result); 
}

#ifdef ELISP_TEST // {{{
TEST_CASE("prelude/mult", "*") {
	Environment testEnv;
	add_globals(testEnv);
	mult_proc proc;
	cell_t* result;
	number_cell* number;
	
	// Two arguments
	cons_cell* twoArgs = makeList({
		new number_cell(1.0),
		new number_cell(2.0)});
	
	result = proc.evalProc(twoArgs, testEnv);
	REQUIRE(result->type == kCellType_number);
	
	number = static_cast<number_cell*>(result);
	REQUIRE(number->value == 2);
	
	// Seven arguments
	cons_cell* sevenArgs = makeList({
		new number_cell(10.0),
		new number_cell(2.0),
		new number_cell(1.0),
		new number_cell(-1.0),
		new number_cell(0.5),
		new number_cell(10.0),
		new number_cell(-5.0)});
	
	result = proc.evalProc(sevenArgs, testEnv);
	REQUIRE(result->type == kCellType_number);
	
	number = static_cast<number_cell*>(result);
	REQUIRE(number->value == 500);
	
	// Nested
	cons_cell* nested = makeList({
		new number_cell(2.0),
		makeList({
			new symbol_cell("*"),
			new number_cell(2.0),
			new number_cell(3.0)}),
		new number_cell(2.0)});
	
	result = proc.evalProc(nested, testEnv);
	REQUIRE(result->type == kCellType_number);
	
	number = static_cast<number_cell*>(result);
	REQUIRE(number->value == 24);
}
#endif // }}}

inline cell_t* div_proc::evalProc(list_cell* args, Environment& env) {
	verifyCell(args, "/");

	cons_cell* currentCell = args;
	double value = getOpValue(env.eval(currentCell->car));
	currentCell = currentCell->cdr;

	if (!currentCell)
		return new number_cell(1.0 / value);

	while (currentCell) {
		value /= getOpValue(env.eval(currentCell->car));
		currentCell = currentCell->cdr;
	}

	return new number_cell(value);
}

#ifdef ELISP_TEST // {{{
TEST_CASE("prelude/div", "/") {
	Environment testEnv;
	add_globals(testEnv);
	div_proc proc;
	cell_t* result;
	number_cell* number;
	
	// One arguments
	cons_cell* oneArgs = makeList({new number_cell(2.0)});
	
	result = proc.evalProc(oneArgs, testEnv);
	REQUIRE(result->type == kCellType_number);
	
	number = static_cast<number_cell*>(result);
	REQUIRE(number->value == 0.5);

	// Two arguments
	cons_cell* twoArgs = makeList({
		new number_cell(4.0),
		new number_cell(2.0)});
	
	result = proc.evalProc(twoArgs, testEnv);
	REQUIRE(result->type == kCellType_number);
	
	number = static_cast<number_cell*>(result);
	REQUIRE(number->value == 2);
	
	// Seven arguments
	cons_cell* sevenArgs = makeList({
		new number_cell(10.0),
		new number_cell(2.0),
		new number_cell(2.5)});
	
	result = proc.evalProc(sevenArgs, testEnv);
	REQUIRE(result->type == kCellType_number);
	
	number = static_cast<number_cell*>(result);
	REQUIRE(number->value == 2);
	
	// Nested
	cons_cell* nested = makeList({
		new number_cell(4.0),
		makeList({
			new symbol_cell("/"),
			new number_cell(2.0),
			new number_cell(1.0)}),
		new number_cell(2.0)});
	
	result = proc.evalProc(nested, testEnv);
	REQUIRE(result->type == kCellType_number);
	
	number = static_cast<number_cell*>(result);
	REQUIRE(number->value == 1);
}
#endif // }}}

inline cell_t* eq_proc::evalProc(list_cell* args, Environment& env) {
	verifyCell(args, "=");

	cons_cell* currentCell = args;

	double value = getOpValue(env.eval(currentCell->car));
	currentCell = currentCell->cdr;
	verifyCell(currentCell, "=");

	// No early out to ensure all args are numbers.
	bool result = true;
	while (currentCell) {
		result = result && (value == getOpValue(env.eval(currentCell->car)));
		currentCell = currentCell->cdr;
	}
	return new bool_cell(result);
}

#ifdef ELISP_TEST // {{{
TEST_CASE("prelude/eq", "=") {
	Environment testEnv;
	add_globals(testEnv);
	eq_proc proc;
	cell_t* result;

	// Two arguments
	cons_cell* twoArgs = makeList({
		new number_cell(2.0),
		new number_cell(2.0)});
	
	result = proc.evalProc(twoArgs, testEnv);
	REQUIRE(result->type == kCellType_bool);
	REQUIRE(static_cast<bool>(result));
	
	// Seven arguments
	cons_cell* sevenArgs = makeList({
		new number_cell(-0.0),
		new number_cell(0.0),
		new number_cell(0.0)});
	
	result = proc.evalProc(sevenArgs, testEnv);
	REQUIRE(result->type == kCellType_bool);
	REQUIRE(static_cast<bool>(result));
	
	// Nested
	cons_cell* nested = makeList({
		new number_cell(4.0),
		makeList({
			new symbol_cell("*"),
			new number_cell(2.0),
			new number_cell(2.0)})});
	
	result = proc.evalProc(nested, testEnv);
	REQUIRE(result->type == kCellType_bool);
	REQUIRE(static_cast<bool>(result));
}
#endif // }}}

inline cell_t* if_proc::evalProc(list_cell* args, Environment& env) {
	verifyCell(args, "if");

	cons_cell* currentCell = args;
	
	cell_t* test = currentCell->car;
	currentCell = currentCell->cdr;
	verifyCell(currentCell, "if");

	cell_t* conseq 	= currentCell->car;
	currentCell = currentCell->cdr;
	verifyCell(currentCell, "if");

	cell_t* alt	= currentCell->car;
	currentCell = currentCell->cdr;

	trueOrDie(currentCell == empty_list, "Too many arguments specified to \"if\"");

	return env.eval((static_cast<bool>(env.eval(test)) ? conseq : alt));
}

#ifdef ELISP_TEST // {{{
TEST_CASE("prelude/if", "if") {
	Environment testEnv;
	add_globals(testEnv);
	if_proc proc;
	cell_t* result;
	number_cell* number;
	cons_cell* args;

	args = makeList({
			makeList({
				new symbol_cell("="),
				new number_cell(1.0),
				new number_cell(1.0)}),
			new number_cell(1.0),
			new number_cell(2.0)});

	result = proc.evalProc(args, testEnv);
	REQUIRE(result->type == kCellType_number);
	number = static_cast<number_cell*>(result);
	REQUIRE(number->value == 1.0);

	args = makeList({
			makeList({
				new symbol_cell("="),
				new number_cell(2.0),
				new number_cell(1.0)}),
			new number_cell(1.0),
			new number_cell(2.0)});

	result = proc.evalProc(args, testEnv);
	REQUIRE(result->type == kCellType_number);
	number = static_cast<number_cell*>(result);
	REQUIRE(number->value == 2.0);

	args = makeList({
			new bool_cell(false),
			new number_cell(1.0),
			new number_cell(2.0)});

	result = proc.evalProc(args, testEnv);
	REQUIRE(result->type == kCellType_number);
	number = static_cast<number_cell*>(result);
	REQUIRE(number->value == 2.0);

	args = makeList({
			new bool_cell(empty_list),
			new number_cell(1.0),
			new number_cell(2.0)});

	result = proc.evalProc(args, testEnv);
	REQUIRE(result->type == kCellType_number);
	number = static_cast<number_cell*>(result);
	REQUIRE(number->value == 2.0);
}
#endif // }}}

inline cell_t* quote_proc::evalProc(list_cell* args, Environment&) {
	verifyCell(args, "quote");
	cell_t* value = args->car;
	trueOrDie(!args->cdr, "Too many arguments specified to \"quote\"");
	return value;
}

#ifdef ELISP_TEST // {{{
TEST_CASE("prelude/quote", "quote") {
	Environment testEnv;
	add_globals(testEnv);
	quote_proc proc;

	cons_cell* args = makeList({makeList({
		new symbol_cell("="),
		new number_cell(1.0),
		new number_cell(1.0)})});

	cell_t* result = proc.evalProc(args, testEnv);
	REQUIRE(result->type == kCellType_cons);
	cons_cell* cons = static_cast<cons_cell*>(result);
	REQUIRE(cons->car->type == kCellType_symbol);
	symbol_cell* equalSymbol = static_cast<symbol_cell*>(cons->car);
	REQUIRE(equalSymbol->identifier == "=");
}
#endif // }}}

inline cell_t* set_proc::evalProc(list_cell* args, Environment& env) {
	verifyCell(args, "set!");
	verifyCell(args->cdr, "set!");

	cell_t* var = args->car;
	cell_t* exp = args->cdr->car;

	trueOrDie(var->type == kCellType_symbol, "set! requires a symbol as its first argument");
	string& id = static_cast<symbol_cell*>(var)->identifier;
	Environment* e = env.find(id);
	trueOrDie(e, "Cannot set undefined variable " + id);

	e->mSymbolMap[id] = env.eval(exp);
	return empty_list;
}

#ifdef ELISP_TEST // {{{
TEST_CASE("prelude/set!", "set!") {
	Environment testEnv;
	add_globals(testEnv);
	testEnv.mSymbolMap["derp"] = new number_cell(1);
	set_proc proc;

	cons_cell* args = makeList({new symbol_cell("derp"), new number_cell(2.0)});

	proc.evalProc(args, testEnv);
	REQUIRE(testEnv.find("derp") == &testEnv);
	cell_t* derpCell = testEnv.get("derp");
	REQUIRE(derpCell->type == kCellType_number);
	number_cell* num = static_cast<number_cell*>(derpCell);
	REQUIRE(num->value == 2);
}
#endif // }}}

inline cell_t* define_proc::evalProc(list_cell* args, Environment& env) {
	// Make sure we got enough arguments.
	verifyCell(args, "define");
	verifyCell(args->cdr, "define");

	cell_t* firstArgument = args->car;
	trueOrDie(firstArgument != empty_list, "No name specified for given function definition.");

	if (firstArgument->type == kCellType_cons) {
		// Defining a function.

		// Get the name of the function we're defining.
		cons_cell* currentParameter = static_cast<cons_cell*>(firstArgument);
		trueOrDie(currentParameter->car->type == kCellType_symbol, "Function name in define declaration must be a symbol.");
		string functionName = static_cast<symbol_cell*>(currentParameter->car)->identifier;

		// Construct a lambda and bind it to the function name.
		lambda_cell* lambda = new lambda_cell;

		// Get the parameter name list if there are any specified.
		currentParameter = currentParameter->cdr;
		while (currentParameter) {
			trueOrDie(currentParameter->car->type == kCellType_symbol,
					"Only symbols can be in the parameter list for a function definition.");
			symbol_cell* parameter = static_cast<symbol_cell*>(currentParameter->car);
			lambda->mParameters.push_back(parameter);
			currentParameter = currentParameter->cdr;
		}

		// Get all the body expressions.
		cons_cell* currentBodyExpr = args->cdr;
		trueOrDie(currentBodyExpr, "At least one body expression is required when defining a function.");
		while (currentBodyExpr) {
			lambda->mBodyExpressions.push_back(currentBodyExpr->car);
			currentBodyExpr = currentBodyExpr->cdr;
		}

		env.mSymbolMap[functionName] = lambda;
	} else if (firstArgument->type == kCellType_symbol) {
		// Defining a variable binding.
		string varName = static_cast<symbol_cell*>(firstArgument)->identifier;
		cell_t* exp = args->cdr->car;

		trueOrDie(args->cdr->cdr == empty_list, "define expects only 2 arguments when defining a variable binding.");

		env.mSymbolMap[varName] = env.eval(exp);
	} else {
		die("Invalid first parameter passed to define.  Expected either a symbol or a list of symbols.");
	}
	return empty_list;
}

#ifdef ELISP_TEST // {{{
TEST_CASE("prelude/define", "define") {
	Environment testEnv;
	add_globals(testEnv);
	define_proc proc;

	cons_cell* args = makeList({new symbol_cell("derp"), new number_cell(2.0)});

	proc.evalProc(args, testEnv);
	REQUIRE(testEnv.find("derp") == &testEnv);
	cell_t* derpCell = testEnv.get("derp");
	REQUIRE(derpCell->type == kCellType_number);
	number_cell* num = static_cast<number_cell*>(derpCell);
	REQUIRE(num->value == 2);
}
#endif // }}}


inline cell_t* lambda_proc::evalProc(list_cell* args, Environment&) {
	//@TODO
	//    elif x[0] == 'lambda':         # (lambda (var*) exp)
	//        (_, vars, exp) = x
	//        return lambda *args: eval(exp, Env(vars, args, env))

	trueOrDie(args != empty_list, "Procedure 'lambda' requires at least 2 arguments, 0 given");
	cons_cell* currentCell = args;


	// Get the paramter list 
	trueOrDie(currentCell->car->type == kCellType_cons, "Second argument to lambda must be a list");
	cons_cell* parameters = static_cast<cons_cell*>(currentCell->car);

	// Move past the list of parameters.
	trueOrDie(currentCell->cdr != empty_list, "Procedure 'lambda' requires at least 2 arguments. 1 given.");
	currentCell = currentCell->cdr;

	// Save the list of body statements.
	cons_cell* listOfBodyStatements = currentCell;

	// Create a lambda and return it.
	lambda_cell* cell = new lambda_cell;
	
	// Add the parameters.
	currentCell = parameters;
	while (currentCell) {
		trueOrDie(currentCell->car->type == kCellType_symbol, "Expected only symbols in lambda parameter list.");
		cell->mParameters.push_back(static_cast<symbol_cell*>(currentCell->car));
		currentCell = currentCell->cdr;
	}

	// Add the body expressions.
	currentCell = listOfBodyStatements;
	while (currentCell) {
		cell->mBodyExpressions.push_back(currentCell->car);
		currentCell = currentCell->cdr;
	}

	return cell;
}

inline cell_t* begin_proc::evalProc(list_cell* args, Environment& env) {
	verifyCell(args, "begin");

	cons_cell* currentCell = args;
	cell_t* value = empty_list;
	while (currentCell) {
		value = env.eval(currentCell->car);
		currentCell = currentCell->cdr;
	}
	return value;
}

inline cell_t* let_proc::evalProc(list_cell* args, Environment& env) {
	verifyCell(args, "let");

	cons_cell* currentCell = args;
	trueOrDie(currentCell->car->type == kCellType_cons, "The second argument to \"let\" must be a list of lists.");
	list_cell* bindings = static_cast<list_cell*>(currentCell->car);

	Environment* newEnv = new Environment(env);

	cons_cell* currentBinding = bindings;
	while (currentBinding) {
		trueOrDie(currentBinding->car->type == kCellType_cons, "The second argument to \"let\" must be a list of lists.");
		cons_cell* currentBindingPair = static_cast<cons_cell*>(currentBinding->car);

		trueOrDie(currentBindingPair->car->type == kCellType_symbol, "First argument in a binding expression must be a symbol");

		symbol_cell* var = static_cast<symbol_cell*>(currentBindingPair->car);
		cell_t* exp = currentBindingPair->cdr->car;

		trueOrDie(!currentBindingPair->cdr->cdr, "Too many arguments in binding expression.");

		newEnv->mSymbolMap[var->identifier] = env.eval(exp);

		currentBinding = currentBinding->cdr;
	}
	currentCell = currentCell->cdr;

	cell_t* returnVal = empty_list;
	while (currentCell) {
		returnVal = newEnv->eval(currentCell->car);
		currentCell = currentCell->cdr;
	}
	return returnVal;
}

inline cell_t* display_proc::evalProc(list_cell* args, Environment& env) {
	cons_cell* currentArgument = args;
	for (; currentArgument; currentArgument = currentArgument->cdr) 
		cout << env.eval(currentArgument->car) << endl;
	return empty_list;
}

inline cell_t* greater_proc::evalProc(list_cell* args, Environment& env) {
	trueOrDie(args && args->cdr, "Function > requires at least two arguments");

	cons_cell* currentArgument = args;

	cell_t* leftCell = env.eval(currentArgument->car);
	cell_t* rightCell;

	trueOrDie(leftCell->type == kCellType_number, "Function > accepts only numerical arguments");

	currentArgument = currentArgument->cdr;

	bool result = true;
	while(currentArgument) {
		rightCell = env.eval(currentArgument->car);
		trueOrDie(rightCell->type == kCellType_number, "Function > accepts only numerical arguments");

		double leftVal = static_cast<number_cell*>(leftCell)->value;
		double rightVal = static_cast<number_cell*>(rightCell)->value;

		result = result && (leftVal > rightVal);
		if (!result)
			break;

		leftCell = rightCell;

		currentArgument = currentArgument->cdr;
	}

	return new bool_cell(result);
}

inline cell_t* less_proc::evalProc(list_cell* args, Environment& env) {
	trueOrDie(args && args->cdr, "Function < requires at least two arguments");

	cons_cell* currentArgument = args;

	cell_t* leftCell = env.eval(currentArgument->car);
	cell_t* rightCell;

	trueOrDie(leftCell->type == kCellType_number, "Function < accepts only numerical arguments");

	currentArgument = currentArgument->cdr;

	bool result = true;
	while(currentArgument) {
		double leftVal = static_cast<number_cell*>(leftCell)->value;

		rightCell = env.eval(currentArgument->car);
		trueOrDie(rightCell->type == kCellType_number, "Function < accepts only numerical arguments");
		double rightVal = static_cast<number_cell*>(rightCell)->value;

		result = result && (leftVal < rightVal);
		if (!result)
			break;

		leftCell = rightCell;

		currentArgument = currentArgument->cdr;
	}

	return new bool_cell(result);
}

