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
#include "internal/reader.h"

//////////////////////////////////////////////////////////////////////////
class Program {
public:
	Program() { add_globals(global_env); }
	
	// Eval the given expression in either the given context or the global context.
	cell_t* eval(cell_t* x) { return global_env.eval(x); }
	cell_t* eval(cell_t* x, Environment& env) { return env.eval(x); }

	// Eval a string of code and give the result as a string.
	string runCode(string inCode) { return to_string(eval(read(inCode))); }

	// Read eval print loop.
	void repl(string prompt = "elisp> ") {
		while (true) {
			cout << prompt;
			string raw_input;
			getline(cin, raw_input);
			cout << runCode(raw_input) << endl << endl;
		}
	}
public:
	Environment global_env;
};

}
