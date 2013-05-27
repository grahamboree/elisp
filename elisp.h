/*
program: (sexpr)*;

sexpr: list
|  atom            {Console.WriteLine("matched sexpr");}
;

list:
'('')'              {Console.WriteLine("matched empty list");}
| '(' members ')'   {Console.WriteLine("matched list");}

;

members: (sexpr)+      {Console.WriteLine("members 1");};

atom: Id               {Console.WriteLine("ID");}
| Num              {Console.WriteLine("NUM");}
;


Num: ( '0' .. '9')+;
Id: ('a' .. 'z' | 'A' .. 'Z')+;
Whitespace : ( ' ' | '\r' '\n' | '\n' | '\t' ) {Skip();};
*/

#pragma once

#include <algorithm>
#include <iostream>
#include <fstream>
#include <iterator>
#include <list>
#include <map>
#include <sstream>
#include <streambuf>
#include <string>
#include <vector>

using namespace std;

#include "data.h"
#include "util.h"
#include "prelude.h"

//////////////////////////////////////////////////////////////////////////
// Utilities
string removeComments(string s) {
	// Look for semicolons.
	string::size_type position = s.find(";");
	while (position != string::npos) {
		// look for newlines after the found semicolon.
		string::size_type lineEnd = s.find_first_of("\n\r");
		//cout << "erasing substring: " << s.substr(s.begin() + position, s.begin() + position + lineEnd)
		s.erase(position, lineEnd - position);
		position = s.find(";");
	}
	return s;
}

Env* global_env = add_globals(new Env());

cell_t* eval(cell_t* x, Env* env = global_env) {
	switch(x->type) {
		case kCellType_symbol: {
			// Symbol lookup in the current environment.
			string& id = static_cast<symbol_cell*>(x)->identifier;
			return env->find(id)->get(id);
		} break;
		case kCellType_cons: {
			// Function call
			list_cell* listcell = static_cast<list_cell*>(x);

			cell_t* callable = eval(listcell->car, env);

			// If the first argument is a symbol, look it up in the current environment.
			if (callable->type == kCellType_symbol) {
				string callableName = static_cast<symbol_cell*>(callable)->identifier;

				Env* enclosingEnvironment = env->find(callableName);
				trueOrDie(enclosingEnvironment, "Undefined function: " + callableName);

				callable = enclosingEnvironment->get(callableName);
			}

			if (callable->type == kCellType_procedure) { // Eval the procedure with the rest of the arguments.
				return static_cast<proc_cell*>(callable)->evalProc(listcell, env);
			} else if (callable->type == kCellType_lambda) { // Eval the lambda with the rest of the arguments.
				return static_cast<lambda_cell*>(callable)->eval(listcell, env);
			} else {
				die("Expected procedure or lambda");
			}
		} break;
		default:{
			// Constant literal
			return x;
		} break;
	}

	return NULL;
}

list<string> tokenize(string s) {
	removeComments(s);
	
	replaceAll(s, "(", " ( ");
	replaceAll(s, ")", " ) ");

	// Split by whitespace
	list<string> tokens;
	istringstream iss(s);
	copy(istream_iterator<string>(iss),
			istream_iterator<string>(),
			back_inserter<list<string> >(tokens));
	return tokens;
}

cell_t* atom(string token) {
	if (token[0] == '#') {
		string::value_type& boolid = token[1];
		bool val = (boolid == 't' || boolid == 'T');
		trueOrDie((val || boolid == 'f' || boolid == 'F'), "Unknown identifier " + token);
		return new bool_cell(val);
	} else if (isNumber(token)) {
		return new number_cell(atof(token.c_str()));
	} else {
		return new symbol_cell(token.c_str());
	}
}

cell_t* read_from(list<string>& inTokens) {
	trueOrDie(!inTokens.empty(), "Unexpected EOF while reading");

	string token = inTokens.front();
	inTokens.pop_front();

	if (token == "(") {
		if (inTokens.front() == ")") {
			return empty_list;
		}

		// Generate a linked list of the elements in the list.
		cons_cell* currentPair;
		list_cell* listatom = currentPair = new cons_cell(read_from(inTokens), NULL);
		//size_t listLength = 1;
		while (inTokens.front() != ")") {
			cons_cell* newCell = new cons_cell(read_from(inTokens), NULL);
			currentPair->cdr = newCell;
			currentPair = newCell;
			//listLength++;
		}

		inTokens.pop_front(); // pop off ")"

		return listatom;
	}
	trueOrDie(token != ")", "Unexpected \")\"");
	return atom(token);
}

cell_t* read(string s) {
	ostringstream ss;
	ss << "(begin " << s << " )";

	list<string> tokens = tokenize(ss.str());

	// A program is a list of sexpressions.  Take all the individual sexpressions and 
	// wrap them in a function taking no arguments.  (semantically the same as a "begin")
	return read_from(tokens);
}

string to_string(cell_t* exp) {
	ostringstream ss;
	if (exp)
		ss << exp;
	else
		ss << "'" << "()";
	return ss.str();
}

string runCode(string inCode) {
	return to_string(eval(read(inCode)));
}

void repl(string prompt = "elisp> ") {
	while (true) {
		cout << prompt;
		string raw_input;
		getline(cin, raw_input);
		cout << runCode(raw_input) << endl << endl;
	}
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


//Symbol = str

//class Env(dict):
//    "An environment: a dict of {'var':val} pairs, with an outer Env."
//    def __init__(self, parms=(), args=(), outer=None):
//        self.update(zip(parms,args))
//        self.outer = outer
//    def find(self, var):
//        "Find the innermost Env where var appears."
//        return self if var in self else self.outer.find(var)

//isa = isinstance

//################ eval

//def eval(x, env=global_env):
//    "Evaluate an expression in an environment."
//    if isa(x, Symbol):             # variable reference
//        return env.find(x)[x]
//    elif not isa(x, list):         # constant literal
//        return x
//    elif x[0] == 'quote':          # (quote exp)
//        (_, exp) = x
//        return exp
//    elif x[0] == 'if':             # (if test conseq alt)
//        (_, test, conseq, alt) = x
//        return eval((conseq if eval(test, env) else alt), env)
//    elif x[0] == 'set!':           # (set! var exp)
//        (_, var, exp) = x
//        env.find(var)[var] = eval(exp, env)
//    elif x[0] == 'define':         # (define var exp)
//        (_, var, exp) = x
//        env[var] = eval(exp, env)
//    elif x[0] == 'lambda':         # (lambda (var*) exp)
//        (_, vars, exp) = x
//        return lambda *args: eval(exp, Env(vars, args, env))
//    elif x[0] == 'begin':          # (begin exp*)
//        for exp in x[1:]:
//            val = eval(exp, env)
//        return val
//    else:                          # (proc exp*)
//        exps = [eval(exp, env) for exp in x]
//        proc = exps.pop(0)
//        return proc(*exps)

//def add_globals(env):
//    "Add some Scheme standard procedures to an environment."
//    import math, operator as op
//    env.update(vars(math)) # sin, sqrt, ...
//    env.update(
//     {'+':op.add, '-':op.sub, '*':op.mul, '/':op.div, 'not':op.not_,
//      '>':op.gt, '<':op.lt, '>=':op.ge, '<=':op.le, '=':op.eq,
//      'equal?':op.eq, 'eq?':op.is_, 'length':len, 'cons':lambda x,y:[x]+y,
//      'car':lambda x:x[0],'cdr':lambda x:x[1:], 'append':op.add,
//      'list':lambda *x:list(x), 'list?': lambda x:isa(x,list),
//      'null?':lambda x:x==[], 'symbol?':lambda x: isa(x, Symbol)})
//    return env

//global_env = add_globals(Env())


//################ parse, read, and user interaction


//parse = read

//def tokenize(s):
//    "Convert a string into a list of tokens."
//    return s.replace('(',' ( ').replace(')',' ) ').split()


//def atom(token):
//    "Numbers become numbers; every other token is a symbol."
//    try: return int(token)
//    except ValueError:
//        try: return float(token)
//        except ValueError:
//            return Symbol(token)

//def read_from(tokens):
//    "Read an expression from a sequence of tokens."
//    if len(tokens) == 0:
//        raise SyntaxError('unexpected EOF while reading')
//    token = tokens.pop(0)
//    if '(' == token:
//        L = []
//        while tokens[0] != ')':
//            L.append(read_from(tokens))
//        tokens.pop(0) # pop off ')'
//        return L
//    elif ')' == token:
//        raise SyntaxError('unexpected )')
//    else:
//        return atom(token)

//def read(s):
//    "Read a Scheme expression from a string."
//    return read_from(tokenize(s))

//def to_string(exp):
//    "Convert a Python object back into a Lisp-readable string."
//    return '('+' '.join(map(to_string, exp))+')' if isa(exp, list) else str(exp)

//def repl(prompt='lis.py> '):
//    "A prompt-read-eval-print loop."
//    while True:
//        val = eval(parse(raw_input(prompt)))
//        if val is not None: print to_string(val)

