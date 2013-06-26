/*
 * The elisp standard prelude
 */

#pragma once

#include "util.h"

struct numerical_proc : public proc_cell {
protected:
	double getOpValue(cell_t* inOp) {
		trueOrDie((inOp->type == kCellType_number), "Error, expected only number arguments");
		return static_cast<number_cell*>(inOp)->value;
	}
};

struct add_proc : public numerical_proc {
	virtual cell_t* evalProc(list_cell* args, Env* env) {
		cons_cell* currentCell = args->cdr; // skip "+"
		verifyCell(currentCell, "+");

		double result = 0.0;
		while(currentCell) {
			result += getOpValue(env->eval(currentCell->car));
			currentCell = currentCell->cdr;
		}

		return new number_cell(result); 
	}
};

#ifdef ELISP_TEST // {{{
TEST_CASE("prelude/add", "Adding two numbers") {
	Env testEnv;
	add_proc a;
	cell_t* result;
	number_cell* number;

	// One argument
	cons_cell* oneArg = makeList({new symbol_cell("+"), new number_cell(1.0)});

	result = a.evalProc(oneArg, &testEnv);
	REQUIRE(result->type == kCellType_number);

	number = static_cast<number_cell*>(result);
	REQUIRE(number->value == 1);

	// Two arguments
	cons_cell* twoArgs = makeList({
			new symbol_cell("+"),
			new number_cell(1.0),
			new number_cell(1.0)});

	result = a.evalProc(twoArgs, &testEnv);
	REQUIRE(result->type == kCellType_number);

	number = static_cast<number_cell*>(result);
	REQUIRE(number->value == 2);

	// Seven arguments
	cons_cell* sevenArgs = makeList({
			new symbol_cell("+"),
			new number_cell(1.0),
			new number_cell(1.0),
			new number_cell(1.0),
			new number_cell(1.0),
			new number_cell(1.0),
			new number_cell(1.0),
			new number_cell(1.0)});

	result = a.evalProc(sevenArgs, &testEnv);
	REQUIRE(result->type == kCellType_number);

	number = static_cast<number_cell*>(result);
	REQUIRE(number->value == 7);
}
#endif // }}}

struct sub_proc : public numerical_proc {
	virtual cell_t* evalProc(list_cell* args, Env* env) {
		cons_cell* currentCell = args->cdr; // skip "-"
		verifyCell(currentCell, "-");

		double result = getOpValue(env->eval(currentCell->car));
		currentCell = currentCell->cdr;

		if (!currentCell)
			return new number_cell(-result);

		while(currentCell) {
			result -= getOpValue(env->eval(currentCell->car));
			currentCell = currentCell->cdr;
		}

		return new number_cell(result); 
	}
};

struct mult_proc : public numerical_proc {
	virtual cell_t* evalProc(list_cell* args, Env* env) {
		cons_cell* currentCell = args->cdr; // skip "*"
		verifyCell(currentCell, "*");

		double result = 1.0;
		while(currentCell) {
			result *= getOpValue(env->eval(currentCell->car));
			currentCell = currentCell->cdr;
		}

		return new number_cell(result); 
	}
};

struct div_proc : public numerical_proc {
	virtual cell_t* evalProc(list_cell* args, Env* env) {
		cons_cell* currentCell = args->cdr; // skip "/"
		verifyCell(currentCell, "/");

		double value = getOpValue(env->eval(currentCell->car));
		currentCell = currentCell->cdr;

		if (!currentCell)
			return new number_cell(1.0 / value);

		while (currentCell) {
			value /= getOpValue(env->eval(currentCell->car));
			currentCell = currentCell->cdr;
		}

		return new number_cell(value);
	}
};

struct eq_proc : public numerical_proc {
	virtual cell_t* evalProc(list_cell* args, Env* env) {
		cons_cell* currentCell = args->cdr; // skip "="
		verifyCell(currentCell, "=");

		double value = getOpValue(env->eval(currentCell->car));
		currentCell = currentCell->cdr;
		verifyCell(currentCell, "=");

		// No early out to ensure all args are numbers.
		bool result = true;
		while (currentCell) {
			//@TODO make this more accurate.
			result = result && (value == getOpValue(env->eval(currentCell->car)));
			currentCell = currentCell->cdr;
		}
		return new bool_cell(result);
	}
};

struct if_proc : public proc_cell {
	virtual cell_t* evalProc(list_cell* args, Env* env) {
		cons_cell* currentCell = args->cdr; verifyCell(currentCell, "if"); // skip "if"

		cell_t* test 	= currentCell->car; currentCell = currentCell->cdr; verifyCell(currentCell, "if");
		cell_t* conseq 	= currentCell->car; currentCell = currentCell->cdr; verifyCell(currentCell, "if");
		cell_t* alt 	= currentCell->car;	currentCell = currentCell->cdr;

		trueOrDie(currentCell == empty_list, "Too many arguments specified to \"if\"");

		return env->eval((cell_to_bool(env->eval(test)) ? conseq : alt));
	}
};

struct quote_proc : public proc_cell { // (quote exp)
	virtual cell_t* evalProc(list_cell* args, Env*) {
		verifyCell(args->cdr, "quote");
		cell_t* value = args->cdr->car;
		trueOrDie(!args->cdr->cdr, "Too many arguments specified to \"quote\"");
		return value;
	}
};

struct set_proc : public proc_cell { // (set! var exp)
	virtual cell_t* evalProc(list_cell* args, Env* env) {
		verifyCell(args->cdr, "set!");
		verifyCell(args->cdr->cdr, "set!");

		cell_t* var = args->cdr->car;
		cell_t* exp = args->cdr->cdr->car;

		trueOrDie(var->type == kCellType_symbol, "Error: set! requires a symbol as its first argument");
		string& id = static_cast<symbol_cell*>(var)->identifier;
		Env* e = env->find(id);
		trueOrDie(e, "Error, cannot set undefined variable " + id);

		e->mSymbolMap[id] = env->eval(exp);
		return empty_list;
	}
};

struct define_proc : public proc_cell { // (define var exp)
	virtual cell_t* evalProc(list_cell* args, Env* env) {
		// Make sure we got enough arguments.
		verifyCell(args->cdr, "define");
		verifyCell(args->cdr->cdr, "define");

		cell_t* firstArgument = args->cdr->car;
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
			cons_cell* currentBodyExpr = args->cdr->cdr;
			trueOrDie(currentBodyExpr, "At least one body expression is required when defining a function.");
			while (currentBodyExpr) {
				lambda->mBodyExpressions.push_back(currentBodyExpr->car);
				currentBodyExpr = currentBodyExpr->cdr;
			}

			env->mSymbolMap[functionName] = lambda;
		} else if (firstArgument->type == kCellType_symbol) {
			// Defining a variable binding.
			string varName = static_cast<symbol_cell*>(firstArgument)->identifier;
			cell_t* exp = args->cdr->cdr->car;

			trueOrDie(args->cdr->cdr->cdr == empty_list, "Error, define expects only 2 arguments when defining a variable binding.");

			env->mSymbolMap[varName] = env->eval(exp);
		} else {
			die("Invalid first parameter passed to define.  Expected either a symbol or a list of symbols.");
		}
		return empty_list;
	}
};

struct lambda_proc : public proc_cell { // (lambda (var*) exp)
	virtual cell_t* evalProc(list_cell* args, Env*) {
		//@TODO
		//    elif x[0] == 'lambda':         # (lambda (var*) exp)
		//        (_, vars, exp) = x
		//        return lambda *args: eval(exp, Env(vars, args, env))

		trueOrDie(args->cdr != empty_list, "Procedure 'lambda' requires at least 2 arguments, 0 given");
		cons_cell* currentCell = args->cdr; // skip "lambda"


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
			trueOrDie(currentCell->car->type == kCellType_symbol, "Error, expected only symbols in lambda parameter list.");
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
};

struct begin_proc : public proc_cell {// (begin exp*)
	virtual cell_t* evalProc(list_cell* args, Env* env) {
		cons_cell* currentCell = args->cdr; // skip "begin"
		verifyCell(currentCell, "begin");

		cell_t* value = empty_list;
		while (currentCell) {
			value = env->eval(currentCell->car);
			currentCell = currentCell->cdr;
		}
		return value;
	}
};

struct let_proc : public proc_cell {
	virtual cell_t* evalProc(list_cell* args, Env* env) {
		cons_cell* currentCell = args->cdr; // skip "let"
		verifyCell(currentCell, "let");

		trueOrDie(currentCell->car->type == kCellType_cons, "The second argument to \"let\" must be a list of lists.");
		list_cell* bindings = static_cast<list_cell*>(currentCell->car);

		Env* newEnv = new Env(env);

		cons_cell* currentBinding = bindings;
		while (currentBinding) {
			trueOrDie(currentBinding->car->type == kCellType_cons, "The second argument to \"let\" must be a list of lists.");
			cons_cell* currentBindingPair = static_cast<cons_cell*>(currentBinding->car);

			trueOrDie(currentBindingPair->car->type == kCellType_symbol, "First argument in a binding expression must be a symbol");

			symbol_cell* var = static_cast<symbol_cell*>(currentBindingPair->car);
			cell_t* exp = currentBindingPair->cdr->car;

			trueOrDie(!currentBindingPair->cdr->cdr, "Too many arguments in binding expression.");

			newEnv->mSymbolMap[var->identifier] = env->eval(exp);

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
};

struct display_proc : public proc_cell {
	virtual cell_t* evalProc(list_cell* args, Env* env) {
		cons_cell* currentArgument = args->cdr;
		for (; currentArgument; currentArgument = currentArgument->cdr) 
			cout << env->eval(currentArgument->car) << endl;
		return empty_list;
	}
};

struct exit_proc : public proc_cell {
	virtual cell_t* evalProc(list_cell*, Env*) {
		exit(0);
	}
};

struct greater_proc : public proc_cell {
	virtual cell_t* evalProc(list_cell* inArgs, Env* inEnv) {
		trueOrDie(inArgs->cdr && inArgs->cdr->cdr, "Error, function > requires at least two arguments");

		cons_cell* currentArgument = inArgs->cdr;

		cell_t* leftCell = inEnv->eval(currentArgument->car);
		cell_t* rightCell;

		trueOrDie(leftCell->type == kCellType_number, "Error, function > accepts only numerical arguments");

		currentArgument = currentArgument->cdr;

		bool result = true;
		while(currentArgument) {
			rightCell = inEnv->eval(currentArgument->car);
			trueOrDie(rightCell->type == kCellType_number, "Error, function > accepts only numerical arguments");

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
};

struct less_proc : public proc_cell {
	virtual cell_t* evalProc(list_cell* inArgs, Env* inEnv) {
		trueOrDie(inArgs->cdr && inArgs->cdr->cdr, "Error, function < requires at least two arguments");

		cons_cell* currentArgument = inArgs->cdr;

		cell_t* leftCell = inEnv->eval(currentArgument->car);
		cell_t* rightCell;

		trueOrDie(leftCell->type == kCellType_number, "Error, function < accepts only numerical arguments");

		currentArgument = currentArgument->cdr;

		bool result = true;
		while(currentArgument) {
			rightCell = inEnv->eval(currentArgument->car);
			trueOrDie(rightCell->type == kCellType_number, "Error, function < accepts only numerical arguments");

			double leftVal = static_cast<number_cell*>(leftCell)->value;
			double rightVal = static_cast<number_cell*>(rightCell)->value;

			result = result && (leftVal < rightVal);
			if (!result)
				break;

			leftCell = rightCell;

			currentArgument = currentArgument->cdr;
		}

		return new bool_cell(result);
	}
};

Env* add_globals(Env* env) {
	env->mSymbolMap["+"] 		= new add_proc;
	env->mSymbolMap["-"] 		= new sub_proc;
	env->mSymbolMap["*"] 		= new mult_proc;
	env->mSymbolMap["/"] 		= new div_proc;
	env->mSymbolMap["="] 		= new eq_proc;
	env->mSymbolMap[">"] 		= new greater_proc;
	env->mSymbolMap["<"] 		= new less_proc;
	env->mSymbolMap["if"] 		= new if_proc;
	env->mSymbolMap["begin"] 	= new begin_proc;
	env->mSymbolMap["define"] 	= new define_proc;
	env->mSymbolMap["lambda"] 	= new lambda_proc;
	env->mSymbolMap["quote"]	= new quote_proc;
	env->mSymbolMap["set!"] 	= new set_proc;
	env->mSymbolMap["let"] 		= new let_proc;
	env->mSymbolMap["display"] 	= new display_proc;
	env->mSymbolMap["exit"] 	= new exit_proc;

	return env;
}
