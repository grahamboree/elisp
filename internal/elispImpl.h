// vim: set foldmethod=marker:
/*
 *
 */

#pragma once

namespace elisp {
	// Assertion functions
	void die(string message) { throw std::logic_error(message); }
	template<typename T> void trueOrDie(T condition, string message) { if (!condition) die(message); }
	shared_ptr<cons_cell> makeList(vector<Cell> list); // Forward-declared because it's used in lambda_cell::eval

	// Cells {{{
	inline cell_t::cell_t(eCellType inType)
	: type(inType)
	{
	}

	inline std::ostream& operator << (std::ostream& os, Cell obj) {
		return (os << static_cast<string>(*obj));
	}

	inline number_cell::operator string() {
		if (valueString.empty()) {
			std::ostringstream ss;
			ss << ((value == (int)value) ? (int)value : value);
			return ss.str();
		}
		return valueString;
	}

	inline cons_cell::cons_cell(Cell inCar, shared_ptr<cons_cell> inCdr)
	: cell_t(kCellType_cons)
	, car(inCar)
	, cdr(inCdr)
	{
	}

	inline Cell cons_cell::GetCar() { return car; }
	inline void cons_cell::SetCar(Cell newCar) { car = newCar; }
	inline shared_ptr<cons_cell> cons_cell::GetCdr() { return cdr; }
	inline void cons_cell::SetCdr(shared_ptr<cons_cell> newCdr) { cdr = newCdr; }

	inline cons_cell::iterator cons_cell::begin() { return iterator(shared_from_this()); }
	inline cons_cell::iterator cons_cell::end() { return iterator(empty_list); }

	cons_cell::operator string() {
		std::ostringstream ss;
		ss << "(";

		iterator it = begin();
		while (it != end()) {
			ss << *it;
			++it;
			if (it != end())
				ss << " ";
		}
		ss << ")";
		return ss.str();
	}

	inline cons_cell::iterator::iterator(shared_ptr<cons_cell> startCell) : currentCell(startCell) {}

	inline cons_cell::iterator& cons_cell::iterator::operator++() {
		currentCell = currentCell->GetCdr();
		return (*this);
	}

	inline cons_cell::iterator cons_cell::iterator::operator++(int) {
		iterator temp = *this;
		currentCell = currentCell->cdr;
		return temp;
	}

	inline bool cons_cell::iterator::operator==(const iterator& other) { return currentCell == other.currentCell; }
	inline bool cons_cell::iterator::operator!=(const iterator& other) { return !((*this) == other); }
	inline Cell cons_cell::iterator::operator *() { return currentCell ? currentCell->GetCar() : nullptr; }

	inline lambda_cell::lambda_cell(Env outerEnv)
	: cell_t(kCellType_lambda)
	, env(outerEnv)
	, mVarargsName(nullptr)
	{
	}

	inline lambda_cell::lambda_cell(Env outerEnv,
			vector<shared_ptr<symbol_cell>>&& inParameters,
			vector<Cell>&& inBodyExpressions,
			shared_ptr<symbol_cell>&& inVarargsName)
	: cell_t(kCellType_lambda)
	, env(outerEnv)
	, mParameters(inParameters)
	, mBodyExpressions(inBodyExpressions)
	, mVarargsName(inVarargsName)
	{
	}

	Cell lambda_cell::eval(shared_ptr<cons_cell> args, Env currentEnv) {
		Env newEnv = std::make_shared<Environment>(env);

		// Match the arguments to the parameters.
		for (auto paramID : mParameters) {
			trueOrDie(args != empty_list, "insufficient arguments provided to function");
			newEnv->mSymbolMap[paramID->GetIdentifier()] = currentEnv->eval(args->GetCar());
			args = args->GetCdr();
		}

		// Either store the rest of the arguments in the variadic name,
		// or error out that there were too many.
		if (mVarargsName) {
			vector<Cell> varargs;
			while (args) {
				varargs.push_back(currentEnv->eval(args->GetCar()));
				args = args->GetCdr();
			}
			newEnv->mSymbolMap[mVarargsName->GetIdentifier()] = makeList(varargs);
		} else if (args) {
			die("too many arguments specified to lambda");
		}

		// Evaluate the body expressions with the new environment.  Return the result of the last body expression.
		Cell returnVal;
		for (auto bodyExpr : mBodyExpressions)
			returnVal = newEnv->eval(bodyExpr);

		return returnVal;
	}

	lambda_cell::operator string() {
		std::ostringstream ss;
		ss << "(lambda (";

		// parameters
		if (mVarargsName and mParameters.empty()) {
			ss << mVarargsName;
		} else {
			bool addspace = false;
			for (auto param : mParameters) {
				if (addspace)
					ss << " ";
				else
					addspace = true;
				ss << param;
			}
			if (mVarargsName)
				ss << " . " << mVarargsName;

			ss << ") ";
		}

		// body expressions
		bool addspace = false;
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

	template<typename T>
	inline bool_cell::bool_cell(T inValue)
	: cell_t(kCellType_bool)
	, value(inValue)
	{
	}

	inline bool_cell::bool_cell(bool inValue)
	:cell_t(kCellType_bool)
	, value(inValue)
	{
	}

	inline bool_cell::operator string() {
		return value ? "#t" : "#f";
	}

	inline number_cell::number_cell(double inValue)
	:cell_t(kCellType_number)
	, value(inValue)
	{
	}

	inline char_cell::char_cell(char inValue)
	:cell_t(kCellType_char)
	, value(inValue)
	{
	}

	inline char_cell::operator string() {
		std::ostringstream ss;
		ss << value;
		return ss.str();
	}

	inline proc_cell::proc_cell(std::function<Cell(shared_ptr<cons_cell>, Env)> procedure)
	: cell_t(kCellType_procedure)
	, mProcedure(procedure)
	{
	}

	inline Cell proc_cell::evalProc(shared_ptr<cons_cell> args, Env env) {
		return mProcedure(args, env);
	};

	inline proc_cell::operator string() {
		return "#procedure";
	}

	// }}}

	// Util {{{
	/**
	 * Converts a Cell to a bool.  Handles nullptr which is used to
	 * indicate an empty list.
	 */
	inline bool cell_to_bool(Cell cell) {
		return (cell != empty_list and (cell->GetType() != kCellType_bool || static_cast<bool_cell*>(cell.get())->GetValue()));
	}

	/**
	 * Replaces all occurances of \p from with \p to in \p str
	 */
	inline void replaceAll(string& str, const string& from, const string& to) {
		string::size_type pos = 0;
		while((pos = str.find(from, pos)) != string::npos) {
			str.replace(pos, from.length(), to);
			pos += to.length();
		}
	}

	/**
	 * Returns \c true if \p inValue is a string representation of a number, \c false otherwise.
	 */
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

	/**
	 * A helper that creates a lisp list given a vector of the list's contents.
	 *
	 * A convenient use-case:
	 * cons_cell* cons_cell = makeList({ new symbol_cell("+"), new number_cell(1), new number_cell(2)});
	 */
	shared_ptr<cons_cell> makeList(vector<Cell> list) {
		shared_ptr<cons_cell> result;
		vector<Cell>::const_reverse_iterator iter;
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

	inline Cell Environment::get(const string& var) {
		return mSymbolMap.find(var)->second;
	}

	Cell Environment::eval(Cell x) {
		trueOrDie(x != nullptr, "Missing procedure.  Original code was most likely (), which is illegal.");
		
		if (x->GetType() == kCellType_symbol) {
			// Symbol lookup in the current environment.
			const string& id = static_cast<symbol_cell*>(x.get())->GetIdentifier();
			return find(id)->get(id);
		} else if (x->GetType() == kCellType_cons) {
			// Function call
			cons_cell* listcell = static_cast<cons_cell*>(x.get());
			Cell callable = this->eval(listcell->GetCar());

			// If the first argument is a symbol, look it up in the current environment.
			if (callable->GetType() == kCellType_symbol) {
				string callableName = static_cast<symbol_cell*>(callable.get())->GetIdentifier();

				Env enclosingEnvironment = find(callableName);
				trueOrDie(enclosingEnvironment, "Undefined function: " + callableName);

				callable = enclosingEnvironment->get(callableName);
			}

			if (callable->GetType() == kCellType_procedure) { 
				// Eval the procedure with the rest of the arguments.
				return static_cast<proc_cell*>(callable.get())->evalProc(listcell->GetCdr(), shared_from_this());
			} else if (callable->GetType() == kCellType_lambda) { 
				// Eval the lambda with the rest of the arguments.
				return static_cast<lambda_cell*>(callable.get())->eval(listcell->GetCdr(), shared_from_this());
			}
			die("Expected procedure or lambda as first element in an sexpression.");
		}
		return x;
	}
	// }}}

	// Prelude {{{
	void add_globals(Env env); // Forward-declared because the test cases need to set up Environments.

	namespace procedures { // {{{
		double GetNumericValue(Cell inOp) {
			trueOrDie((inOp->GetType() == kCellType_number), "Expected only number arguments");
			return static_cast<number_cell*>(inOp.get())->GetValue();
		}

		void verifyCell(shared_ptr<cons_cell> inCell, string methodName) {
			trueOrDie(inCell, "Insufficient arguments provided to " + methodName + ".");
		}

		Cell add(shared_ptr<cons_cell> args, Env env) {
			verifyCell(args, "+");
			
			double result = 0.0;
			for (auto arg : *args)
				result += GetNumericValue(env->eval(arg));
			
			return std::make_shared<number_cell>(result);
		};

#ifdef ELISP_TEST // {{{
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
#endif // }}}

		Cell sub(shared_ptr<cons_cell> args, Env env) {
			verifyCell(args, "-");

			shared_ptr<cons_cell> currentCell = args;
			auto it = args->begin();

			double result = GetNumericValue(env->eval(*it));
			++it;

			if (it == args->end())
				return std::make_shared<number_cell>(-result);

			for (;it != args->end(); ++it)
				result -= GetNumericValue(env->eval(*it));

			return std::make_shared<number_cell>(result); 
		}

#ifdef ELISP_TEST // {{{
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
#endif // }}}

		Cell mult(shared_ptr<cons_cell> args, Env env) {
			verifyCell(args, "*");

			double result = 1.0;
			for (auto arg : *args)
				result *= GetNumericValue(env->eval(arg));

			return std::make_shared<number_cell>(result);
		}

#ifdef ELISP_TEST // {{{
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
#endif // }}}

		Cell div(shared_ptr<cons_cell> args, Env env) {
			verifyCell(args, "/");

			auto it = args->begin();
			double value = GetNumericValue(env->eval(*it));
			++it;

			if (it == args->end())
				return std::make_shared<number_cell>(1.0 / value);

			for (;it != args->end(); ++it)
				value /= GetNumericValue(env->eval(*it));

			return std::make_shared<number_cell>(value);
		}

#ifdef ELISP_TEST // {{{
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
#endif // }}}

		Cell eq(shared_ptr<cons_cell> args, Env env) {
			verifyCell(args, "=");

			auto it = args->begin();

			double value = GetNumericValue(env->eval(*it));
			++it;
			verifyCell(args->GetCdr(), "=");

			// No early out to ensure all args are numbers.
			bool result = true;
			for (; it != args->end(); ++it)
				result = result && (value == GetNumericValue(env->eval(*it)));
			return std::make_shared<bool_cell>(result);
		}

#ifdef ELISP_TEST // {{{
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
#endif // }}}

		Cell if_then_else(shared_ptr<cons_cell> args, Env env) {
			auto currentCell = args;
			
			verifyCell(currentCell, "if");
			Cell test = currentCell->GetCar();
			currentCell = currentCell->GetCdr();

			verifyCell(currentCell, "if");
			Cell conseq = currentCell->GetCar();
			currentCell = currentCell->GetCdr();

			verifyCell(currentCell, "if");
			Cell alt = currentCell->GetCar();
			currentCell = currentCell->GetCdr();

			trueOrDie(currentCell == empty_list, "Too many arguments specified to \"if\"");

			return env->eval((cell_to_bool(env->eval(test)) ? conseq : alt));
		}

#ifdef ELISP_TEST // {{{
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
#endif // }}}

#if 0
		Cell quote(shared_ptr<cons_cell> args, Env) {
			verifyCell(args, "quote");
			auto value = args->car;
			trueOrDie(!args->cdr, "Too many arguments specified to \"quote\"");
			return value;
		}

#ifdef ELISP_TEST // {{{
		TEST_CASE("prelude/quote", "quote") {
			Env testEnv = std::make_shared<Environment>();
			add_globals(testEnv);

			Cell result = quote(makeList({makeList({
				std::make_shared<symbol_cell>("="),
				std::make_shared<number_cell>(1.0),
				std::make_shared<number_cell>(1.0)})}), testEnv);
			REQUIRE(result->GetType() == kCellType_cons);
			auto cons = std::static_pointer_cast<cons_cell>(result);
			REQUIRE(cons->car->GetType() == kCellType_symbol);
			auto equalSymbol = std::static_pointer_cast<symbol_cell>(cons->car);
			REQUIRE(equalSymbol->GetIdentifier() == "=");
		}
#endif // }}}

		Cell set(shared_ptr<cons_cell> args, Env env) {
			verifyCell(args, "set!");
			verifyCell(args->cdr, "set!");

			auto var = args->car;
			auto exp = args->cdr->car;

			trueOrDie(var->GetType() == kCellType_symbol, "set! requires a symbol as its first argument");
			const auto& id = std::static_pointer_cast<symbol_cell>(var)->GetIdentifier();
			auto e = env->find(id);
			trueOrDie(e, "Cannot set undefined variable " + id);

			e->mSymbolMap[id] = env->eval(exp);
			return empty_list;
		}

#ifdef ELISP_TEST // {{{
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
#endif // }}}

		Cell define(shared_ptr<cons_cell> args, Env env) {
			// Make sure we got enough arguments.
			verifyCell(args, "define");
			verifyCell(args->cdr, "define");

			auto firstArgument = args->car;
			trueOrDie(firstArgument != empty_list, "No name specified for given function definition.");

			if (firstArgument->GetType() == kCellType_cons) {
				// Defining a function.

				// Get the name of the function we're defining.
				auto currentParameter = std::static_pointer_cast<cons_cell>(firstArgument);
				trueOrDie(currentParameter->car->GetType() == kCellType_symbol, "Function name in define declaration must be a symbol.");
				auto functionName = std::static_pointer_cast<symbol_cell>(currentParameter->car)->GetIdentifier();

				// The elements of the lamgbda
				vector<shared_ptr<symbol_cell>> parameters; 
				vector<Cell> bodyExpressions; 
				shared_ptr<symbol_cell> varargsName; 

				// Get the parameter name list if there are any specified.
				currentParameter = currentParameter->cdr;
				bool varargs = false;
				while (currentParameter) {
					trueOrDie(currentParameter->car->GetType() == kCellType_symbol,
							"Only symbols can be in the parameter list for a function definition.");
					auto parameter = std::static_pointer_cast<symbol_cell>(currentParameter->car);
					if (varargs) {
						varargsName = parameter;
						trueOrDie(currentParameter->cdr == empty_list, "Expected only one varargs identifier following '.' in parameter list of lambda definition");
					} else if (parameter->GetIdentifier() == ".") {
						varargs = true;
						trueOrDie(currentParameter->cdr != empty_list, "Expected varargs identifier following '.' in parameter list of lambda definition");
					} else {
						parameters.push_back(parameter);
					}
					currentParameter = currentParameter->cdr;
				}

				// Get all the body expressions.
				auto currentBodyExpr = args->cdr;
				trueOrDie(currentBodyExpr, "At least one body expression is required when defining a function.");
				while (currentBodyExpr) {
					bodyExpressions.push_back(currentBodyExpr->car);
					currentBodyExpr = currentBodyExpr->cdr;
				}

				// Construct a lambda and bind it to the function name.
				env->mSymbolMap[functionName] =
					std::make_shared<lambda_cell>(env, std::move(parameters), std::move(bodyExpressions), std::move(varargsName));
			} else if (firstArgument->GetType() == kCellType_symbol) {
				// Defining a variable binding.
				auto varName = std::static_pointer_cast<symbol_cell>(firstArgument)->GetIdentifier();
				auto exp = args->cdr->car;

				trueOrDie(args->cdr->cdr == empty_list, "define expects only 2 arguments when defining a variable binding.");

				env->mSymbolMap[varName] = env->eval(exp);
			} else {
				die("Invalid first parameter passed to define.  Expected either a symbol or a list of symbols.");
			}
			return empty_list;
		}

#ifdef ELISP_TEST // {{{
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
#endif // }}}

		Cell lambda(shared_ptr<cons_cell> args, Env env) {
			trueOrDie(args != empty_list, "Procedure 'lambda' requires at least 2 arguments, 0 given");
			auto currentCell = args;

			// The elements of the lamgbda
			vector<shared_ptr<symbol_cell>> lambdaParameters; 
			vector<Cell> bodyExpressions; 
			shared_ptr<symbol_cell> varargsName; 
			
			// Get the paramter list 
			if (currentCell->car->GetType() == kCellType_cons) {
				auto parameters = std::static_pointer_cast<cons_cell>(currentCell->car);
				
				// Add the parameters.
				auto currentParameter = parameters;
				bool varargs = false;
				while (currentParameter) {
					trueOrDie(currentParameter->car->GetType() == kCellType_symbol, "Expected only symbols in lambda parameter list.");
					auto symbolCell = std::static_pointer_cast<symbol_cell>(currentParameter->car);
					if (varargs) {
						varargsName = symbolCell;
						trueOrDie(currentParameter->cdr == empty_list, "Only one identifier can follow a '.' in the parameter list of a lambda expression.");
					} else if (symbolCell->GetIdentifier() == ".") {
						varargs = true;
						trueOrDie(currentParameter->cdr != empty_list, "Expected varargs name following '.' in lambda expression.");
					} else {
						lambdaParameters.push_back(symbolCell);
					}
					currentParameter = currentParameter->cdr;
				}
			} else if (currentCell->car->GetType() == kCellType_symbol) {
				varargsName = std::static_pointer_cast<symbol_cell>(currentCell->car);
			} else {
				die("Second argument to a lambda expression must be either a symbol or a list of symbols.");
			}

			// Move past the list of parameters.
			trueOrDie(currentCell->cdr != empty_list, "Procedure 'lambda' requires at least 2 arguments. 1 given.");
			currentCell = currentCell->cdr;

			// Add the body expressions.
			while (currentCell) {
				bodyExpressions.push_back(currentCell->car);
				currentCell = currentCell->cdr;
			}

			// Create a lambda and return it.
			return std::make_shared<lambda_cell>(env, std::move(lambdaParameters), std::move(bodyExpressions), std::move(varargsName));
		}

		Cell begin(shared_ptr<cons_cell> args, Env env) {
			verifyCell(args, "begin");

			auto currentCell = args;
			auto value = env->eval(currentCell->car);
			currentCell = currentCell->cdr;
			while (currentCell) {
				value = env->eval(currentCell->car);
				currentCell = currentCell->cdr;
			}
			return value;
		}

		Cell let(shared_ptr<cons_cell> args, Env env) {
			verifyCell(args, "let");

			auto currentCell = args;
			trueOrDie(currentCell->car->GetType() == kCellType_cons, "The first argument to \"let\" must be a list of lists.");
			auto bindings = std::static_pointer_cast<cons_cell>(currentCell->car);

			Env newEnv = std::make_shared<Environment>(env);

			auto currentBinding = bindings;
			while (currentBinding) {
				trueOrDie(currentBinding->car->GetType() == kCellType_cons, "The first argument to \"let\" must be a list of lists.");
				auto currentBindingPair = std::static_pointer_cast<cons_cell>(currentBinding->car);

				trueOrDie(currentBindingPair->car->GetType() == kCellType_symbol, "First argument in a binding expression must be a symbol");

				auto var = std::static_pointer_cast<symbol_cell>(currentBindingPair->car);
				Cell exp = currentBindingPair->cdr->car;

				trueOrDie(!currentBindingPair->cdr->cdr, "Too many arguments in binding expression.");

				newEnv->mSymbolMap[var->GetIdentifier()] = env->eval(exp);

				currentBinding = currentBinding->cdr;
			}
			currentCell = currentCell->cdr;

			Cell returnVal = empty_list;
			while (currentCell) {
				returnVal = newEnv->eval(currentCell->car);
				currentCell = currentCell->cdr;
			}
			return returnVal;
		}

		Cell display(shared_ptr<cons_cell> args, Env env) {
			auto currentArgument = args;
			for (; currentArgument; currentArgument = currentArgument->cdr) 
				std::cout << env->eval(currentArgument->car) << std::endl;
			return empty_list;
		}

		Cell greater(shared_ptr<cons_cell> args, Env env) {
			trueOrDie(args && args->cdr, "Function > requires at least two arguments");

			auto currentArgument = args;

			Cell leftCell = env->eval(currentArgument->car);
			Cell rightCell;

			trueOrDie(leftCell->GetType() == kCellType_number, "Function > accepts only numerical arguments");

			currentArgument = currentArgument->cdr;

			bool result = true;
			while(currentArgument) {
				rightCell = env->eval(currentArgument->car);
				trueOrDie(rightCell->GetType() == kCellType_number, "Function > accepts only numerical arguments");

				double leftVal = GetNumericValue(leftCell);
				double rightVal = GetNumericValue(rightCell);

				result = result && (leftVal > rightVal);
				if (!result)
					break;

				leftCell = rightCell;

				currentArgument = currentArgument->cdr;
			}

			return std::make_shared<bool_cell>(result);
		}

		Cell less(shared_ptr<cons_cell> args, Env env) {
			trueOrDie(args && args->cdr, "Function < requires at least two arguments");

			auto currentArgument = args;

			Cell leftCell = env->eval(currentArgument->car);
			Cell rightCell;

			trueOrDie(leftCell->GetType() == kCellType_number, "Function < accepts only numerical arguments");

			currentArgument = currentArgument->cdr;

			bool result = true;
			while(currentArgument) {
				double leftVal = GetNumericValue(leftCell);

				rightCell = env->eval(currentArgument->car);
				trueOrDie(rightCell->GetType() == kCellType_number, "Function < accepts only numerical arguments");
				double rightVal = GetNumericValue(rightCell);

				result = result && (leftVal < rightVal);
				if (!result)
					break;

				leftCell = rightCell;

				currentArgument = currentArgument->cdr;
			}

			return std::make_shared<bool_cell>(result);
		}

		Cell exit(shared_ptr<cons_cell>, Env) {
			::exit(0);
		}
#endif
	} // }}}
	
	/**
	 * Adds the standard prelude functions to the global environment.
	 */
	void add_globals(Env env) {
		using namespace procedures;
		env->mSymbolMap.insert({
			{"+", 		std::make_shared<proc_cell>(add)},
			{"-", 		std::make_shared<proc_cell>(sub)},
			{"*", 		std::make_shared<proc_cell>(mult)},
			{"/", 		std::make_shared<proc_cell>(div)},
			{"=", 		std::make_shared<proc_cell>(eq)},
			{"if", 		std::make_shared<proc_cell>(if_then_else)},
			/*
			{"quote", 	std::make_shared<proc_cell>(quote)},
			{"set!",	std::make_shared<proc_cell>(set)},
			{"define", 	std::make_shared<proc_cell>(define)},
			{"lambda", 	std::make_shared<proc_cell>(lambda)},
			{"begin", 	std::make_shared<proc_cell>(begin)},
			{"let",		std::make_shared<proc_cell>(let)},
			{"display", std::make_shared<proc_cell>(display)},
			{"<", 		std::make_shared<proc_cell>(less)},
			{">", 		std::make_shared<proc_cell>(greater)},
			{"exit", 	std::make_shared<proc_cell>(exit)},*/
		});
	}
	// }}}

	// TokenStream, writer, reader, and Program {{{
	TokenStream::TokenStream(std::istream& stream)
	: is(stream)
	{
	}

	string TokenStream::nextToken() {
		if (line.empty() and !std::getline(is, line))
			return "";

		std::smatch match;
		if (std::regex_search(line, match, reg)) {
			trueOrDie(match.prefix().str() == "", "unknown characters: " + match.prefix().str());

			string matchStr = match[1].str();
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
	Cell Program::atom(const string& token) {
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
			n->SetValueString(token);
			return std::static_pointer_cast<cell_t>(n);
		}

		return std::static_pointer_cast<cell_t>(std::make_shared<symbol_cell>(token));
	}

	/**
	 * Returns a list of top-level expressions.
	 */
	std::vector<Cell> Program::read(TokenStream& stream) {
		std::vector<std::vector<Cell>> exprStack; // The current stack of nested list expressions.
		exprStack.emplace_back(); // top-level scope

		for (string token = stream.nextToken(); !token.empty(); token = stream.nextToken()) {
			if (token == "(") {
				// Push a new scope onto the stack.
				exprStack.emplace_back();
			} else if (token == ")") {
				// Pop the current scope off the stack and add it as a list to its parent scope.
				trueOrDie(exprStack.size() > 1, "Unexpected ) while reading");
				Cell listexpr = makeList(exprStack.back());
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
	inline std::vector<Cell> Program::read(string s) {
		std::istringstream iss(s);
		TokenStream tokStream(iss);
		return read(tokStream);
	}

	inline string Program::to_string(Cell exp) {
		if (exp == nullptr)
			return "'()";

		std::ostringstream ss;
		ss << exp;
		return ss.str();
	}

	inline Program::Program() {
		global_env = std::make_shared<Environment>();
		add_globals(global_env);
	}

	/// Eval a string of code and give the result as a string.
	inline string Program::runCode(string inCode) {
		std::istringstream iss(inCode);
		TokenStream tokStream(iss);
		return runCode(tokStream);
	}

	/// Given a stream, read and eval the code read from the stream.
	string Program::runCode(TokenStream& stream) {
		using std::cerr;
		using std::endl;

		try {
			Cell result = nullptr;
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
	void Program::repl(string prompt) {
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

	/** Tokenizer regex */
	const std::regex TokenStream::reg(
			R"(\s*)" // skip whitespace
		   "("
				",@|" 				// splice unquote
				R"([\('`,\)]|)" 			// parens, quoting symbols
				R"("(?:[\\].|[^\\"])*"|)" // string literals
				";.*|" 				// comments
				R"([^\s('"`,;)]*))" // identifiers
			")");
	// }}}
}
