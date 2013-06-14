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
class Program {
public:
	Program();
	~Program();
	
	// Eval the given expression in either the given context or the global context.
	inline cell_t* 	eval(cell_t* x) {return global_env->eval(x);}
	inline cell_t* 	eval(cell_t* x, Env* env) { if (!env) env = global_env; return env->eval(x);}
	inline string 	runCode(string inCode) { return to_string(eval(read(inCode))); }
	void 			repl(string prompt = "elisp> ");

	static string 		removeComments(string s);
	static list<string> tokenize(string s);
	static cell_t* 		atom(string token);
	static cell_t* 		read_from(list<string>& inTokens);
	static cell_t* 		read(string s);
	static string 		to_string(cell_t* exp);

public:
	Env* global_env;
};

// Program class Impl {{{1
//////////////////////////////////////////////////////////////////////////
Program::Program()
: global_env(add_globals(new Env()))
{
}

//////////////////////////////////////////////////////////////////////////
Program::~Program() {
	delete global_env;
	global_env = NULL;
}

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
list<string> Program::tokenize(string s) {
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

////////////////////////////////////////////////////////////////////////////////
cell_t* Program::atom(string token) {
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

////////////////////////////////////////////////////////////////////////////////
cell_t* Program::read_from(list<string>& inTokens) {
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
		while (inTokens.front() != ")") {
			cons_cell* newCell = new cons_cell(read_from(inTokens), NULL);
			currentPair->cdr = newCell;
			currentPair = newCell;
		}

		inTokens.pop_front(); // pop off ")"

		return listatom;
	}
	trueOrDie(token != ")", "Unexpected \")\"");
	return atom(token);
}

////////////////////////////////////////////////////////////////////////////////
cell_t* Program::read(string s) {
	ostringstream ss;
	ss << "(begin " << s << " )";

	list<string> tokens = tokenize(ss.str());

	// A program is a list of sexpressions.  Take all the individual sexpressions and 
	// wrap them in a function taking no arguments.  (semantically the same as a "begin")
	return read_from(tokens);
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
