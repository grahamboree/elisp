/*
 *
 */

#pragma once

#include <iostream>
#include <map>
#include <string>
#include <sstream>
#include <vector>

namespace elisp {
using std::back_inserter;
using std::cin;
using std::copy;
using std::cout;
using std::endl;
using std::istream_iterator;
using std::istringstream;
using std::map;
using std::ostream;
using std::ostringstream;
using std::string;
using std::vector;

#include "internal/Assert.h"
#include "internal/Cells.h"
#include "internal/util.h"
#include "internal/Environment.h"
#include "internal/data.h"
#include "internal/prelude.h"

//////////////////////////////////////////////////////////////////////////
class Program {
public:
	static string 		removeComments(string s);
	static vector<string> tokenize(string s);
	static cell_t* 		atom(string token);
	static cell_t* 		read_from(vector<string>& inTokens);
	static cell_t* 		read(string s);
	static string 		to_string(cell_t* exp);

	Program() { add_globals(global_env); }
	
	// Eval the given expression in either the given context or the global context.
	inline cell_t* 	eval(cell_t* x) { return global_env.eval(x); }
	inline cell_t* 	eval(cell_t* x, Environment& env) { return env.eval(x); }
	inline string 	runCode(string inCode) { return to_string(eval(read(inCode))); }
	void 			repl(string prompt = "elisp> ");
public:
	Environment global_env;
};

// Program class Impl {{{1
//////////////////////////////////////////////////////////////////////////
string Program::removeComments(string s) {
	// Look for semicolons.
	string::size_type position = s.find_first_of(";");
	while (position != string::npos) {
		// look for newlines after the found semicolon.
		string::size_type lineEnd = s.find_first_of("\n\r");
		//cout << "erasing substring: " << s.substr(s.begin() + position, s.begin() + position + lineEnd)
		s.erase(position, lineEnd - position);
		position = s.find_first_of(";");
	}
	return s;
}

////////////////////////////////////////////////////////////////////////////////
vector<string> Program::tokenize(string s) {
	removeComments(s);
	
	replaceAll(s, "(", " ( ");
	replaceAll(s, ")", " ) ");

	// Split by whitespace
	vector<string> tokens;
	istringstream iss(s);
	copy(istream_iterator<string>(iss),
			istream_iterator<string>(),
			back_inserter<vector<string> >(tokens));
	return tokens;
}

////////////////////////////////////////////////////////////////////////////////
cell_t* Program::atom(string token) {
	if (token[0] == '#') {
		string::value_type& boolid = token[1];
		bool val = (boolid == 't' || boolid == 'T');
		trueOrDie((val || boolid == 'f' || boolid == 'F'), "Unknown identifier " + token);
		return new bool_cell(val);
	} else if (isNumber(token)) {
		number_cell* n = new number_cell(atof(token.c_str()));
		n->valueString = token;
		return n;
	} else {
		return new symbol_cell(token.c_str());
	}
}

////////////////////////////////////////////////////////////////////////////////
cell_t* Program::read_from(vector<string>& inTokens) {
	trueOrDie(!inTokens.empty(), "Unexpected EOF while reading");

	string token = inTokens.back();
	inTokens.pop_back();

	if (token == "(") {
		if (inTokens.back() == ")") {
			return empty_list;
		}

		// Generate a linked list of the elements in the list.
		cons_cell* currentPair;
		list_cell* listatom = currentPair = new cons_cell(read_from(inTokens), NULL);
		while (inTokens.back() != ")") {
			cons_cell* newCell = new cons_cell(read_from(inTokens), NULL);
			currentPair->cdr = newCell;
			currentPair = newCell;
		}

		inTokens.pop_back();

		return listatom;
	}
	trueOrDie(token != ")", "Unexpected \")\"");
	return atom(token);
}

////////////////////////////////////////////////////////////////////////////////
cell_t* Program::read(string s) {
	ostringstream ss;
	ss << "(begin " << s << " )";

	vector<string> tokens = tokenize(ss.str());

	// Tokens are reversed for efficient access and removal.
	vector<string> rtokens(tokens.rbegin(), tokens.rend());

	// A program is a list of sexpressions.  Take all the individual sexpressions and 
	// wrap them in a function taking no arguments.  (semantically the same as a "begin")
	
	return read_from(rtokens);
}

////////////////////////////////////////////////////////////////////////////////
string Program::to_string(cell_t* exp) {
	ostringstream ss;
	if (exp)
		ss << exp;
	else
		ss << "'" << "()";
	return ss.str();
}

////////////////////////////////////////////////////////////////////////////////
void Program::repl(string prompt) {
	while (true) {
		cout << prompt;
		string raw_input;
		getline(cin, raw_input);
		cout << runCode(raw_input) << endl << endl;
	}
}
// 1}}}
}
