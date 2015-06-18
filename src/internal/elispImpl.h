// vim: set foldmethod=marker:
//
//  elispImpl.h
//  Implementation of elisp
//
//  Created by Graham Pentheny on 12/21/14.
//  Copyright (c) 2014 Graham Pentheny. All rights reserved.
//

#pragma once

namespace elisp {
	// Prelude {{{
	void add_globals(Env env); // Forward-declared because the test cases need to set up Environments.

	namespace procedures { // {{{
		double GetNumericValue(Cell inOp) {
			trueOrDie((inOp->GetType() == kCellType_number), "Expected only number arguments");
			return static_cast<number_cell*>(inOp.get())->GetValue();
		}

		void verifyCell(shared_ptr<cons_cell> inCell, string functionName) {
			trueOrDie(inCell, "Insufficient arguments provided to " + functionName + ".");
		}

		void verifyCell(Cell inCell, string functionName) {
			if (inCell and inCell->GetType() != kCellType_cons)
				throw std::runtime_error("Attempting to verify non cons-cell.");
			
			trueOrDie(inCell, "Insufficient arguments provided to " + functionName + ".");
		}

		Cell add(shared_ptr<cons_cell> args, Env env) {
			verifyCell(args, "+");
			
			double result = 0.0;
			for (auto arg : *args)
				result += GetNumericValue(env->eval(arg));
			
			return std::make_shared<number_cell>(result);
		};


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

		Cell mult(shared_ptr<cons_cell> args, Env env) {
			verifyCell(args, "*");

			double result = 1.0;
			for (auto arg : *args)
				result *= GetNumericValue(env->eval(arg));

			return std::make_shared<number_cell>(result);
		}

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

		Cell if_then_else(shared_ptr<cons_cell> args, Env env) {
			auto it = args->begin();
			
			trueOrDie(it != args->end(), "Insufficient arguments provided to \"if\"");
			Cell test = *it;
			++it;

			trueOrDie(it != args->end(), "Insufficient arguments provided to \"if\"");
			Cell conseq = *it;
			++it;

			trueOrDie(it != args->end(), "Insufficient arguments provided to \"if\"");
			Cell alt = *it;
			++it;

			trueOrDie(it == args->end(), "Too many arguments specified to \"if\"");

			return env->eval((cell_to_bool(env->eval(test)) ? conseq : alt));
		}

		Cell quote(shared_ptr<cons_cell> args, Env) {
			verifyCell(args, "quote");
			auto value = args->GetCar();
			trueOrDie(!args->GetCdr(), "Too many arguments specified to \"quote\"");
			return value;
		}

		Cell set(shared_ptr<cons_cell> args, Env env) {
			verifyCell(args, "set!");
			verifyCell(args->GetCdr(), "set!");

			auto it = args->begin();
			auto var = *it;
			++it;
			auto exp = *it;

			trueOrDie(var->GetType() == kCellType_symbol, "set! requires a symbol as its first argument");
			const auto& id = std::static_pointer_cast<symbol_cell>(var)->GetIdentifier();
			auto e = env->find(id);
			trueOrDie(e, "Cannot set undefined variable " + id);

			e->mSymbolMap[id] = env->eval(exp);
			return empty_list;
		}

		Cell define(shared_ptr<cons_cell> args, Env env) {
			// Make sure we got enough arguments.
			verifyCell(args, "define");
			verifyCell(args->GetCdr(), "define");

			auto it = args->begin();
			trueOrDie(it != args->end(), "No name specified for given function definition.");

			if ((*it)->GetType() == kCellType_cons) {
				// Defining a function.

				// Get the name of the function we're defining.
				auto params = std::static_pointer_cast<cons_cell>(*it);
				auto paramIt = params->begin();

				trueOrDie((*paramIt)->GetType() == kCellType_symbol, "Function name in define declaration must be a symbol.");
				auto functionName = std::static_pointer_cast<symbol_cell>(*paramIt)->GetIdentifier();

				// The elements of the lamgbda
				vector<shared_ptr<symbol_cell>> parameters; 
				vector<Cell> bodyExpressions; 
				shared_ptr<symbol_cell> varargsName; 

				// Get the parameter name list if there are any specified.
				++paramIt;
				bool varargs = false;

				for (;paramIt != params->end(); ++paramIt) {
					trueOrDie((*paramIt)->GetType() == kCellType_symbol,
							"Only symbols can be in the parameter list for a function definition.");
					auto parameter = std::static_pointer_cast<symbol_cell>(*paramIt);
					if (varargs) {
						varargsName = parameter;

						auto next = paramIt;
						++next;
						trueOrDie(next == params->end(), "Expected only one varargs identifier following '.' in parameter list of lambda definition");
					} else if (parameter->GetIdentifier() == ".") {
						varargs = true;

						auto next = paramIt;
						++next;
						trueOrDie(next != params->end(), "Expected varargs identifier following '.' in parameter list of lambda definition");
					} else {
						parameters.push_back(parameter);
					}
				}

				// Get all the body expressions.
				++it;
				trueOrDie(it != args->end(), "At least one body expression is required when defining a function.");
				for (; it != args->end(); ++it)
					bodyExpressions.push_back(*it);

				// Construct a lambda and bind it to the function name.
				env->mSymbolMap[functionName] =
					std::make_shared<lambda_cell>(env, std::move(parameters), std::move(bodyExpressions), std::move(varargsName));
			} else if ((*it)->GetType() == kCellType_symbol) {
				// Defining a variable binding.
				auto varName = std::static_pointer_cast<symbol_cell>(*it)->GetIdentifier();
				++it;
				env->mSymbolMap[varName] = env->eval(*it);

				trueOrDie(++it == empty_list, "define expects only 2 arguments when defining a variable binding.");
			} else {
				die("Invalid first parameter passed to define.  Expected either a symbol or a list of symbols.");
			}
			return empty_list;
		}

		Cell lambda(shared_ptr<cons_cell> args, Env env) {
			trueOrDie(args != empty_list, "Procedure 'lambda' requires at least 2 arguments, 0 given");
			auto it = args->begin();

			// The elements of the lamgbda
			vector<shared_ptr<symbol_cell>> lambdaParameters; 
			vector<Cell> bodyExpressions; 
			shared_ptr<symbol_cell> varargsName; 
			
			// Get the paramter list 
			if ((*it)->GetType() == kCellType_cons) {
				auto parameters = std::static_pointer_cast<cons_cell>(*it);
				
				// Add the parameters.
				auto paramIt = parameters->begin();
				bool varargs = false;
				for (; paramIt != parameters->end(); ++paramIt) {
					trueOrDie((*paramIt)->GetType() == kCellType_symbol, "Expected only symbols in lambda parameter list.");
					auto symbolCell = std::static_pointer_cast<symbol_cell>(*paramIt);
					if (varargs) {
						varargsName = symbolCell;
						auto next = paramIt;
						++next;
						trueOrDie(next == parameters->end(), "Only one identifier can follow a '.' in the parameter list of a lambda expression.");
					} else if (symbolCell->GetIdentifier() == ".") {
						varargs = true;
						auto next = paramIt;
						++next;
						trueOrDie(next != parameters->end(), "Expected varargs name following '.' in lambda expression.");
					} else {
						lambdaParameters.push_back(symbolCell);
					}
				}
			} else if ((*it)->GetType() == kCellType_symbol) {
				varargsName = std::static_pointer_cast<symbol_cell>(*it);
			} else {
				die("Second argument to a lambda expression must be either a symbol or a list of symbols.");
			}

			// Move past the list of parameters.
			++it;
			trueOrDie(it != args->end(), "Procedure 'lambda' requires at least 2 arguments. 1 given.");

			// Add the body expressions.
			for (;it != args->end(); ++it)
				bodyExpressions.push_back(*it);

			// Create a lambda and return it.
			return std::make_shared<lambda_cell>(env, std::move(lambdaParameters), std::move(bodyExpressions), std::move(varargsName));
		}

		Cell begin(shared_ptr<cons_cell> args, Env env) {
			verifyCell(args, "begin");

			Cell value = empty_list;
			for (auto arg : *args) 
				value = env->eval(arg);

			return value;
		}

		Cell let(shared_ptr<cons_cell> args, Env env) {
			verifyCell(args, "let");

			auto it = args->begin();
			trueOrDie((*it)->GetType() == kCellType_cons, "The first argument to \"let\" must be a list of lists.");
			auto bindings = std::static_pointer_cast<cons_cell>(*it);

			Env newEnv = std::make_shared<Environment>(env);

			for (auto binding : *bindings) {
				trueOrDie(binding->GetType() == kCellType_cons, "The first argument to \"let\" must be a list of lists.");
				auto currentBindingPair = std::static_pointer_cast<cons_cell>(binding);
				auto bindingIt = currentBindingPair->begin();

				trueOrDie((*bindingIt)->GetType() == kCellType_symbol, "First argument in a binding expression must be a symbol");

				auto var = std::static_pointer_cast<symbol_cell>(*bindingIt);
				++bindingIt;
				Cell exp = *bindingIt;

				++bindingIt;
				trueOrDie(bindingIt == currentBindingPair->end(), "Too many arguments in binding expression.");

				newEnv->mSymbolMap[var->GetIdentifier()] = env->eval(exp);
			}

			++it;

			Cell returnVal = empty_list;
			for (; it != args->end(); ++it)
				returnVal = newEnv->eval(*it);

			return returnVal;
		}

		Cell display(shared_ptr<cons_cell> args, Env env) {
			for (auto arg : *args)
				std::cout << env->eval(arg) << std::endl;
			return empty_list;
		}

		Cell greater(shared_ptr<cons_cell> args, Env env) {
			trueOrDie(args && args->GetCdr(), "Function > requires at least two arguments");

			auto it = args->begin();

			Cell leftCell = env->eval(*it);
			Cell rightCell;

			trueOrDie(leftCell->GetType() == kCellType_number, "Function > accepts only numerical arguments");

			++it;

			bool result = true;
			for (;it != args->end(); ++it) {
				rightCell = env->eval(*it);
				trueOrDie(rightCell->GetType() == kCellType_number, "Function > accepts only numerical arguments");

				double leftVal = GetNumericValue(leftCell);
				double rightVal = GetNumericValue(rightCell);

				result = result && (leftVal > rightVal);
				if (!result) break;

				leftCell = rightCell;
			}

			return std::make_shared<bool_cell>(result);
		}

		Cell less(shared_ptr<cons_cell> args, Env env) {
			trueOrDie(args && args->GetCdr(), "Function < requires at least two arguments");

			auto it = args->begin();

			Cell leftCell = env->eval(*it);
			Cell rightCell;

			trueOrDie(leftCell->GetType() == kCellType_number, "Function < accepts only numerical arguments");

			++it;

			bool result = true;
			for (; it != args->end(); ++it) {
				double leftVal = GetNumericValue(leftCell);

				rightCell = env->eval(*it);
				trueOrDie(rightCell->GetType() == kCellType_number, "Function < accepts only numerical arguments");
				double rightVal = GetNumericValue(rightCell);

				result = result && (leftVal < rightVal);
				if (!result) break;

				leftCell = rightCell;
			}

			return std::make_shared<bool_cell>(result);
		}

		Cell exit(shared_ptr<cons_cell>, Env) {
			::exit(0);
		}

		Cell cons(shared_ptr<cons_cell> args, Env env) {
			auto it = args->begin();
			trueOrDie(it != args->end(), "Cons expects exactly 2 arguments");
			Cell car = env->eval(*it);

			++it;
			trueOrDie(it != args->end(), "Cons expects exactly 2 arguments");
			Cell cdr = env->eval(*it);

			++it;
			trueOrDie(it == args->end(), "Cons expects exactly 2 arguments");

			return std::make_shared<cons_cell>(car, cdr);
		}

		Cell car(shared_ptr<cons_cell> args, Env env) {
			auto it = args->begin();
			trueOrDie(it != args->end(), "car expects exactly 1 argument");
			Cell cell = env->eval(*it);

			++it;
			trueOrDie(it == args->end(), "car expects exactly 1 argument");

			trueOrDie(cell, "Cannot get the car of an empty list");
			trueOrDie(cell->GetType() == kCellType_cons, "Cannot get the car of something that's not a cons cell");
			
			return std::static_pointer_cast<cons_cell>(cell)->GetCar();
		}

		Cell cdr(shared_ptr<cons_cell> args, Env env) {
			auto it = args->begin();
			trueOrDie(it != args->end(), "cdr expects exactly 1 argument");
			Cell cell = env->eval(*it);

			++it;
			trueOrDie(it == args->end(), "Cdr expects exactly 2 arguments");

			trueOrDie(cell, "Cannot get the cdr of an empty list");
			trueOrDie(cell->GetType() == kCellType_cons, "Cannot get the cdr of something that's not a cons cell");
			
			return std::static_pointer_cast<cons_cell>(cell)->GetCdr();
		}

		Cell length(shared_ptr<cons_cell> args, Env env) {
			trueOrDie(args, "Function less requires at least one arguments");
			auto it = args->begin();
			Cell leftCell = env->eval(*it);
			trueOrDie(leftCell->GetType() == kCellType_cons, "Function length accepts only list arguments");
			return std::make_shared<number_cell>(listLength(leftCell));
		}
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
			{"quote", 	std::make_shared<proc_cell>(quote)},
			{"set!",	std::make_shared<proc_cell>(set)},
			{"define", 	std::make_shared<proc_cell>(define)},
			{"lambda", 	std::make_shared<proc_cell>(lambda)},
			{"begin", 	std::make_shared<proc_cell>(begin)},
			{"let",		std::make_shared<proc_cell>(let)},
			{"display", std::make_shared<proc_cell>(display)},
			{">", 		std::make_shared<proc_cell>(greater)},
			{"<", 		std::make_shared<proc_cell>(less)},
			{"exit", 	std::make_shared<proc_cell>(exit)},
			{"cons", 	std::make_shared<proc_cell>(cons)},
			{"car", 	std::make_shared<proc_cell>(car)},
			{"cdr", 	std::make_shared<proc_cell>(cdr)},
			{"length", 	std::make_shared<proc_cell>(length)},
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

	// Expand `x => 'x; `,x => x; `(,@x y) => (append x y) 
	Cell expand_quasiquote(Cell x) {
		if (x->GetType() != kCellType_cons) {
			return makeList({std::make_shared<symbol_cell>("quote"), x});
		}

		Cell car = std::static_pointer_cast<cons_cell>(x)->GetCar();
		trueOrDie(
			car->GetType() == kCellType_symbol &&
			std::static_pointer_cast<symbol_cell>(car)->GetIdentifier() == "unquote-splicing",
			"can't splice here");
		
		if (car->GetType() == kCellType_symbol &&
			std::static_pointer_cast<symbol_cell>(car)->GetIdentifier() == "unquote") {
			trueOrDie(
				x->GetType() == kCellType_cons &&
				listLength(std::static_pointer_cast<cons_cell>(x)) == 2,
				"");

			auto list = std::static_pointer_cast<cons_cell>(x);
			return list->GetCdr()->GetCar();
		}
		/*
		if x[0] is _unquote:
			require(x, len(x) == 2)
			return x[1]
		elif is_pair(x[0]) and x[0][0] is _unquotesplicing:
			require(x[0], len(x[0]) == 2)
			return [_append, x[0][1], expand_quasiquote(x[1:])]
		else:
			return [_cons, expand_quasiquote(x[0]), expand_quasiquote(x[1:])]
		*/
		return nullptr;
	}

	Cell Program::expand(Cell x, bool topLevel) {
		/*
		require(x, x != [])                # () => Error
		if not isinstance(x, list):        # constant => unchanged
			return x
		elif x[0] is _quote:            # (quote exp)
			require(x, len(x) == 2)
			return x
		elif x[0] is _if:
			if len(x) == 3:
				x = x + [None]  # (if t c) => (if t c None)
			require(x, len(x) == 4)
			return map(expand, x)
		elif x[0] is _set:
			require(x, len(x) == 3)
			var = x[1]                    # (set! non-var exp) => Error
			require(x, isinstance(var, Symbol), "can set! only a symbol")
			return [_set, var, expand(x[2])]
		elif x[0] is _define or x[0] is _definemacro:
			require(x, len(x) >= 3)
			_def, v, body = x[0], x[1], x[2:]
			if isinstance(v, list) and v:    # (define (f args) body)
				f, args = v[0], v[1:]        # => (define f (lambda (args) body))
				return expand([_def, f, [_lambda, args]+body])
			else:
				require(x, len(x) == 3)        # (define non-var/list exp) => Error
				require(x, isinstance(v, Symbol), "can define only a symbol")
				exp = expand(x[2])
				if _def is _definemacro:
					require(x, toplevel, "define-macro only allowed at top level")
					proc = eval(exp)
					require(x, callable(proc), "macro must be a procedure")
					macro_table[v] = proc    # (define-macro v proc)
					return None              # => None; add v:proc to macro_table
				return [_define, v, exp]
		elif x[0] is _begin:
			if len(x) == 1:
				return None        # (begin) => None
			else:
				return [expand(xi, toplevel) for xi in x]
		elif x[0] is _lambda:                    # (lambda (x) e1 e2)
			require(x, len(x) >= 3)              # => (lambda (x) (begin e1 e2))
			vars, body = x[1], x[2:]
			require(x, (isinstance(vars, list) and all(isinstance(v, Symbol) for v in vars))
					or isinstance(vars, Symbol), "illegal lambda argument list")
			exp = body[0] if len(body) == 1 else [_begin] + body
			return [_lambda, vars, expand(exp)]
		elif x[0] is _quasiquote:                # `x => expand_quasiquote(x)
			require(x, len(x) == 2)
			return expand_quasiquote(x[1])
		elif isinstance(x[0], Symbol) and x[0] in macro_table:
			return expand(macro_table[x[0]](*x[1:]), toplevel)  # (m arg...)
		else:                                    # => macroexpand if m isa macro
			return map(expand, x)                # (f arg...) => expand each
		*/
		
	}

	/** Tokenizer regex */
	const std::regex TokenStream::reg(
			R"(\s*)" // skip whitespace
		   "("
				",@|" 				// splice unquote
				R"([\('`,\)]|)" 	// parens, quoting symbols
				R"("(?:[\\].|[^\\"])*"|)" // string literals
				";.*|" 				// comments
				R"([^\s('"`,;)]*))" // identifiers
			")");
	// }}}
}
