// vim: set foldmethod=marker:
/*
 *
 */

#pragma once

namespace elisp {
	// Cells {{{
	inline cell_t::cell_t(eCellType inType)
	: type(inType)
	{
	}

	std::ostream& operator << (std::ostream& os, shared_ptr<cell_t> obj) {
		return (os << static_cast<string>(*obj));
	}

	number_cell::operator string() {
		if (valueString.empty()) {
			std::ostringstream ss;
			ss << ((value == (int)value) ? (int)value : value);
			return ss.str();
		}
		return valueString;
	}

	void proc_cell::verifyCell(shared_ptr<cons_cell> inCell, string methodName) {
		trueOrDie(inCell, "Insufficient arguments provided to " + methodName + ".");
	}

	cons_cell::cons_cell(shared_ptr<cell_t> inCar, shared_ptr<cons_cell> inCdr)
	: cell_t(kCellType_cons)
	, car(inCar)
	, cdr(inCdr)
	{
	}

	cons_cell::operator string() {
		std::ostringstream ss;
		ss << "(";

		shared_ptr<cons_cell> currentCell = shared_from_this();
		while (currentCell != nullptr) {
			ss << currentCell->car;
			currentCell = currentCell->cdr;
			if (currentCell)
				ss << " ";
		}
		ss << ")";
		return ss.str();
	}

	shared_ptr<cell_t> lambda_cell::eval(shared_ptr<cons_cell> args, Env currentEnv) {
		Env newEnv = std::make_shared<Environment>(env);

		// Match the arguments to the parameters.
		for (auto paramID : mParameters) {
			trueOrDie(args != empty_list, "insufficient arguments provided to function");
			newEnv->mSymbolMap[paramID->identifier] = currentEnv->eval(args->car);
			args = args->cdr;
		}

		// Evaluate the body expressions with the new environment.  Return the result of the last body expression.
		shared_ptr<cell_t> returnVal;
		for (auto bodyExpr : mBodyExpressions)
			returnVal = newEnv->eval(bodyExpr);

		return returnVal;
	}

	lambda_cell::operator string() {
		std::ostringstream ss;
		ss << "(lambda (";

		// parameters
		bool addspace = false;
		for (auto param : mParameters) {
			if (addspace)
				ss << " ";
			else
				addspace = true;
			ss << param;
		}
		ss << ") ";


		// body expressions
		addspace = false;
		for (auto bodyExpr : mBodyExpressions) {
			if (addspace)
				ss << " ";
			else
				addspace = true;
			ss << bodyExpr;
		}
		ss << ")";

		return ss.str();
	}
	// }}}

	////////////////////////////////////////////////////////////////////////////////
	// Util {{{
	bool cell_to_bool(shared_ptr<cell_t> cell) {
		return (cell != empty_list and (cell->type != kCellType_bool || static_cast<bool_cell*>(cell.get())->value));
	}

	void replaceAll(string& str, const string& from, const string& to) {
		string::size_type pos = 0;
		while((pos = str.find(from, pos)) != string::npos) {
			str.replace(pos, from.length(), to);
			pos += to.length();
		}
	}

	bool isNumber(string inValue) {
		string::const_iterator it = inValue.begin();
		bool hasRadix = false;
		bool hasDigit = false;

		if (!inValue.empty() && ((hasDigit = (isdigit(*it) != 0)) || *it == '-')) {
			++it;
			for (;it != inValue.end(); ++it) {
				bool digit = (isdigit(*it) != 0);
				hasDigit = hasDigit || digit;
				if (!(digit || (!hasRadix && (hasRadix = (*it == '.')))))
					break;
			}
			return hasDigit && it == inValue.end();
		}
		return false;
	}

	shared_ptr<cons_cell> makeList(vector<shared_ptr<cell_t>> list) {
		shared_ptr<cons_cell> result;
		vector<shared_ptr<cell_t>>::const_reverse_iterator iter;
		for (iter = list.rbegin(); iter != list.rend(); ++iter)
			result = std::make_shared<cons_cell>(*iter, result);
		return result;
	}
	// }}}
	
	// Environment {{{
	inline Environment::Environment() {}
	inline Environment::Environment(Env inOuter) :outer(inOuter) {}

	inline Env Environment::find(const string& var) {
		if (mSymbolMap.find(var) != mSymbolMap.end())
			return shared_from_this();
		trueOrDie(outer, "Undefined symbol " + var);
		return outer->find(var);
	}

	inline shared_ptr<cell_t> Environment::get(const string& var) {
		return mSymbolMap.find(var)->second;
	}

	inline shared_ptr<cell_t> Environment::eval(shared_ptr<cell_t> x) {
		trueOrDie(x != nullptr, "Missing procedure.  Original code was most likely (), which is illegal.");
		
		if (x->type == kCellType_symbol) {
			// Symbol lookup in the current environment.
			string& id = static_cast<symbol_cell*>(x.get())->identifier;
			return find(id)->get(id);
		} else if (x->type == kCellType_cons) {
			// Function call
			cons_cell* listcell = static_cast<cons_cell*>(x.get());
			shared_ptr<cell_t> callable = this->eval(listcell->car);

			// If the first argument is a symbol, look it up in the current environment.
			if (callable->type == kCellType_symbol) {
				string callableName = static_cast<symbol_cell*>(callable.get())->identifier;

				Env enclosingEnvironment = find(callableName);
				trueOrDie(enclosingEnvironment, "Undefined function: " + callableName);

				callable = enclosingEnvironment->get(callableName);
			}

			if (callable->type == kCellType_procedure) { 
				// Eval the procedure with the rest of the arguments.
				return static_cast<proc_cell*>(callable.get())->evalProc(listcell->cdr, shared_from_this());
			} else if (callable->type == kCellType_lambda) { 
				// Eval the lambda with the rest of the arguments.
				return static_cast<lambda_cell*>(callable.get())->eval(listcell->cdr, shared_from_this());
			}
			die("Expected procedure or lambda as first element in an sexpression.");
		}
		return x;
	}
	// }}}

	// Prelude {{{
	struct numerical_proc : public proc_cell {
	protected:
		double getOpValue(shared_ptr<cell_t> inOp) {
			trueOrDie((inOp->type == kCellType_number), "Expected only number arguments");
			return static_cast<number_cell*>(inOp.get())->value;
		}
	};

	struct add_proc : public numerical_proc { virtual shared_ptr<cell_t> evalProc(shared_ptr<cons_cell> args, Env env); };
	/*struct sub_proc : public numerical_proc { virtual cell_t* evalProc(cons_cell* args, Env env); };
	struct mult_proc: public numerical_proc { virtual cell_t* evalProc(cons_cell* args, Env env); };
	struct div_proc : public numerical_proc { virtual cell_t* evalProc(cons_cell* args, Env env); };
	struct eq_proc 	: public numerical_proc { virtual cell_t* evalProc(cons_cell* args, Env env); };
	struct if_proc 		: public proc_cell  { virtual cell_t* evalProc(cons_cell* args, Env env); };
	struct quote_proc 	: public proc_cell  { virtual cell_t* evalProc(cons_cell* args, Env); };
	struct set_proc 	: public proc_cell  { virtual cell_t* evalProc(cons_cell* args, Env env); };
	struct define_proc 	: public proc_cell  { virtual cell_t* evalProc(cons_cell* args, Env env); };
	struct lambda_proc 	: public proc_cell  { virtual cell_t* evalProc(cons_cell* args, Env env); };
	struct begin_proc 	: public proc_cell  { virtual cell_t* evalProc(cons_cell* args, Env env); };
	struct let_proc 	: public proc_cell  { virtual cell_t* evalProc(cons_cell* args, Env env); };
	struct display_proc : public proc_cell  { virtual cell_t* evalProc(cons_cell* args, Env env); };
	struct exit_proc 	: public proc_cell  { virtual cell_t* evalProc(cons_cell*, Env) { exit(0); } };
	struct greater_proc : public proc_cell  { virtual cell_t* evalProc(cons_cell* args, Env env); };
	struct less_proc 	: public proc_cell  { virtual cell_t* evalProc(cons_cell* args, Env env); };*/

	void add_globals(Env env) {
		env->mSymbolMap["+"] 		= std::make_shared< add_proc     >();
		/*env.mSymbolMap["-"] 		= std::make_shared< sub_proc     >();
		env.mSymbolMap["*"] 		= std::make_shared< mult_proc    >();
		env.mSymbolMap["/"] 		= std::make_shared< div_proc     >();
		env.mSymbolMap["="] 		= std::make_shared< eq_proc      >();
		env.mSymbolMap[">"] 		= std::make_shared< greater_proc >();
		env.mSymbolMap["<"] 		= std::make_shared< less_proc    >();
		env.mSymbolMap["if"] 		= std::make_shared< if_proc      >();
		env.mSymbolMap["begin"] 	= std::make_shared< begin_proc   >();
		env.mSymbolMap["define"] 	= std::make_shared< define_proc  >();
		env.mSymbolMap["lambda"] 	= std::make_shared< lambda_proc  >();
		env.mSymbolMap["quote"]		= std::make_shared< quote_proc   >();
		env.mSymbolMap["set!"] 		= std::make_shared< set_proc     >();
		env.mSymbolMap["let"] 		= std::make_shared< let_proc     >();
		env.mSymbolMap["display"] 	= std::make_shared< display_proc >();
		env.mSymbolMap["exit"] 		= std::make_shared< exit_proc    >();*/
	}

	inline shared_ptr<cell_t> add_proc::evalProc(shared_ptr<cons_cell> args, Env env) {
		verifyCell(args, "+");
		
		shared_ptr<cons_cell> currentCell = args;
		double result = 0.0;
		while(currentCell) {
			result += getOpValue(env->eval(currentCell->car));
			currentCell = currentCell->cdr;
		}
		
		return std::static_pointer_cast<cell_t>(std::make_shared<number_cell>(result));
	}

#if 0
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

	inline cell_t* sub_proc::evalProc(cons_cell* args, Environment& env) {
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

	inline cell_t* mult_proc::evalProc(cons_cell* args, Environment& env) {
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

	inline cell_t* div_proc::evalProc(cons_cell* args, Environment& env) {
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

	inline cell_t* eq_proc::evalProc(cons_cell* args, Environment& env) {
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
		REQUIRE(cell_to_bool(result));
		
		// Seven arguments
		cons_cell* sevenArgs = makeList({
			new number_cell(-0.0),
			new number_cell(0.0),
			new number_cell(0.0)});
		
		result = proc.evalProc(sevenArgs, testEnv);
		REQUIRE(result->type == kCellType_bool);
		REQUIRE(cell_to_bool(result));
		
		// Nested
		cons_cell* nested = makeList({
			new number_cell(4.0),
			makeList({
				new symbol_cell("*"),
				new number_cell(2.0),
				new number_cell(2.0)})});
		
		result = proc.evalProc(nested, testEnv);
		REQUIRE(result->type == kCellType_bool);
		REQUIRE(cell_to_bool(result));
	}\

#endif // }}}

	inline cell_t* if_proc::evalProc(cons_cell* args, Environment& env) {
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

		return env.eval((cell_to_bool(env.eval(test)) ? conseq : alt));
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

	inline cell_t* quote_proc::evalProc(cons_cell* args, Environment&) {
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

	inline cell_t* set_proc::evalProc(cons_cell* args, Environment& env) {
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

	inline cell_t* define_proc::evalProc(cons_cell* args, Environment& env) {
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
			lambda_cell* lambda = new lambda_cell(&env);

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


	inline cell_t* lambda_proc::evalProc(cons_cell* args, Environment& env) {
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
		lambda_cell* cell = new lambda_cell(&env);
		
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

	inline cell_t* begin_proc::evalProc(cons_cell* args, Environment& env) {
		verifyCell(args, "begin");

		cons_cell* currentCell = args;
		cell_t* value = empty_list;
		while (currentCell) {
			value = env.eval(currentCell->car);
			currentCell = currentCell->cdr;
		}
		return value;
	}

	inline cell_t* let_proc::evalProc(cons_cell* args, Environment& env) {
		verifyCell(args, "let");

		cons_cell* currentCell = args;
		trueOrDie(currentCell->car->type == kCellType_cons, "The second argument to \"let\" must be a list of lists.");
		cons_cell* bindings = static_cast<list_cell*>(currentCell->car);

		Env newEnv = std::make_shared<Environment>(env);

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

	inline cell_t* display_proc::evalProc(cons_cell* args, Environment& env) {
		cons_cell* currentArgument = args;
		for (; currentArgument; currentArgument = currentArgument->cdr) 
			std::cout << env.eval(currentArgument->car) << std::endl;
		return empty_list;
	}

	inline cell_t* greater_proc::evalProc(cons_cell* args, Environment& env) {
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

	inline cell_t* less_proc::evalProc(cons_cell* args, Environment& env) {
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
	// }}}
#endif

	// TokenStream, writer, reader, and Program {{{
	TokenStream::TokenStream(std::istream& stream) : is(stream) {}

	std::string TokenStream::nextToken() {
		if (line.empty() and !std::getline(is, line))
			return "";

		std::smatch match;
		if (std::regex_search(line, match, reg)) {
			trueOrDie(match.prefix().str() == "", "unknown characters: " + match.prefix().str());

			std::string matchStr = match[1].str();
			line = match.suffix().str();
			if (matchStr.empty() or matchStr[0] == ';')
				return nextToken();
			else
				return matchStr;
				
		} else {
			trueOrDie(false, "Unknown characters: " + line);
		}
		return "";
	}

	/**
	 * Given a string token, creates the atom it represents
	 */
	shared_ptr<cell_t> Program::atom(const std::string& token) {
		if (token[0] == '#') {
			const auto& boolid = token[1];
			bool val = (boolid == 't' || boolid == 'T');
			trueOrDie((val || boolid == 'f' || boolid == 'F') && token.size() == 2, "Unknown identifier " + token);
			return std::static_pointer_cast<cell_t>(std::make_shared<bool_cell>(val));
		} else if (token[0] == '"') {
			return std::static_pointer_cast<cell_t>(std::make_shared<string_cell>(token));
		} else if (isNumber(token)) {
			std::istringstream iss(token);
			double value = 0.0;
			iss >> value;
			auto n = std::make_shared<number_cell>(value);
			n->valueString = token;
			return std::static_pointer_cast<cell_t>(n);
		}

		return std::static_pointer_cast<cell_t>(std::make_shared<symbol_cell>(token));
	}

	/**
	 * Returns a list of top-level expressions.
	 */
	std::vector<shared_ptr<cell_t>> Program::read(TokenStream& stream) {
		std::vector<std::vector<shared_ptr<cell_t>>> exprStack; // The current stack of nested list expressions.
		exprStack.emplace_back(); // top-level scope

		for (std::string token = stream.nextToken(); !token.empty(); token = stream.nextToken()) {
			if (token == "(") {
				// Push a new scope onto the stack.
				exprStack.emplace_back();
			} else if (token == ")") {
				// Pop the current scope off the stack and add it as a list to its parent scope.
				trueOrDie(exprStack.size() > 1, "Unexpected ) while reading");
				shared_ptr<cell_t> listexpr = makeList(exprStack.back());
				exprStack.pop_back();
				exprStack.back().push_back(listexpr);
			} else {
				exprStack.back().push_back(atom(token));
			}
		}

		trueOrDie(exprStack.size() == 1, "Unexpected EOF while reading");
		return exprStack.back();
	}

	/**
	 * Returns a list of top-level expressions.
	 */
	std::vector<shared_ptr<cell_t>> Program::read(string s) {
		std::istringstream iss(s);
		TokenStream tokStream(iss);
		return read(tokStream);
	}

	std::string Program::to_string(shared_ptr<cell_t> exp) {
		if (exp == nullptr)
			return "'()";

		std::ostringstream ss;
		ss << exp;
		return ss.str();
	}

	Program::Program() { global_env = std::make_shared<Environment>(); add_globals(global_env); }

	/// Eval a string of code and give the result as a string.
	inline string Program::runCode(string inCode) {
		std::istringstream iss(inCode);
		TokenStream tokStream(iss);
		return runCode(tokStream);
	}

	/// Given a stream, read and eval the code read from the stream.
	inline string Program::runCode(TokenStream& stream) {
		using std::cerr;
		using std::endl;

		try {
			shared_ptr<cell_t> result = nullptr;
			for (auto expr : read(stream))
				result = global_env->eval(expr);
			return to_string(result);
		} catch (const std::logic_error& e) {
			// logic_error's are thrown for invalid code.
			cerr << "[ERROR]\t" << e.what() << endl;
		} catch (const std::exception& e) {
			// runtime_error's are internal errors at no fault of the user.
			cerr << endl << endl << "--[SYSTEM ERROR]--" << endl << endl << e.what() << endl << endl;
		} catch (...) {
			cerr << endl << endl << "--[SYSTEM ERROR]--" << endl << endl << "An unkown error occured" << endl << endl;
		}
		return "";
	}

	/// Read eval print loop.
	inline void Program::repl(string prompt) {
		using std::cout;
		using std::endl;

		while (true) {
			cout << prompt;

			string raw_input;
			if (!std::getline(std::cin, raw_input)) {
				cout << endl << endl;
				break;
			}

			if (raw_input.empty() or raw_input.find_first_not_of(" \t") == string::npos)
				continue;
			cout << runCode(raw_input) << endl;
		}
	}
	// }}}
}

