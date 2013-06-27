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
using std::exception;
using std::istream_iterator;
using std::istringstream;
using std::logic_error;
using std::map;
using std::ostream;
using std::ostringstream;
using std::runtime_error;
using std::string;
using std::vector;

#include "internal/assert.h"
#include "internal/Cells.h"
#include "internal/util.h"
#include "internal/Environment.h"
#include "internal/data.h"
#include "internal/prelude.h"
#include "internal/reader.h"

class Program {
public:
	Program() { add_globals(global_env); }
	
	// Eval the given expression in either the given context or the global context.
	cell_t* eval(cell_t* x) { return global_env.eval(x); }

	// Eval a string of code and give the result as a string.
	string runCode(string inCode) { return to_string(eval(read(inCode))); }

	// Read eval print loop.
	void repl(string prompt = "elisp> ") {
		while (true) {
			try {
				cout << prompt;
				string raw_input;
				if (!std::getline(cin, raw_input)) {
					cout << endl << endl;
					break;
				}
				if (raw_input.empty() or raw_input.find_first_not_of(" \t") == string::npos)
					continue;
				cout << runCode(raw_input) << endl;
			} catch (const logic_error& e) {
				// logic_error's are thrown for invalid code.
				cout << "[ERROR]\t" << e.what() << endl;
			} catch (const exception& e) {
				// runtime_error's are internal errors at no fault of the user.
				cout << endl << endl << "--[SYSTEM ERROR]--" << endl << endl << e.what() << endl << endl;
			} catch (...) {
				cout << "\n\n--[ERROR]--\t\tAn unkown error occured" << endl;
			}
		}
	}
public:
	Environment global_env;
};

}
