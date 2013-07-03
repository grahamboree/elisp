/*
 *
 */

#pragma once

#include <iostream>
#include <map>
#include <regex>
#include <string>
#include <sstream>
#include <vector>

namespace elisp {
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

	// Eval a string of code and give the result as a string.
	string runCode(string inCode) {
		std::istringstream iss(inCode);
		TokenStream tokStream(iss);
		return runCode(tokStream);
	}

	string runCode(TokenStream& stream) {
		using std::cerr;
		using std::endl;

		try {
			cell_t* result = nullptr;
			for (auto expr : read(stream))
				result = global_env.eval(expr);
			return to_string(result);
		} catch (const logic_error& e) {
			// logic_error's are thrown for invalid code.
			cerr << "[ERROR]\t" << e.what() << endl;
			return "";
		} catch (const exception& e) {
			// runtime_error's are internal errors at no fault of the user.
			cerr << endl << endl << "--[SYSTEM ERROR]--" << endl << endl << e.what() << endl << endl;
			return "";
		} catch (...) {
			cerr << "\n\n--[ERROR]--\t\tAn unkown error occured" << endl;
			return "";
		}
	}

	// Read eval print loop.
	void repl(string prompt = "elisp> ") {
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
public:
	Environment global_env;
};

}
