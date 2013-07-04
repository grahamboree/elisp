/*
 *
 */

#pragma once

// STL dependencies
#include <iostream>
#include <map>
#include <memory>
#include <regex>
#include <string>
#include <sstream>
#include <vector>

namespace elisp {
	using std::string;
	using std::vector;

	// Assertion functions
	void die(string message) { throw std::logic_error(message); }
	void trueOrDie(bool condition, string message) { if (!condition) die(message); }

	// Data types in elisp
	enum eCellType {
		kCellType_bool,
		kCellType_number,
		kCellType_char,
		kCellType_string,
		kCellType_symbol,
		kCellType_cons,
		kCellType_pair,
		kCellType_vector,
		kCellType_procedure,
		kCellType_lambda
	};

	/// Cell type base class
	struct cell_t {
		eCellType type;
		
		virtual operator string() = 0;
		operator bool();
	protected:
		cell_t(eCellType inType);
	};

	class Environment {
		Environment* outer;
	public:
		std::map<string, cell_t*> mSymbolMap;

		Environment();
		Environment(Environment& inOuter);
		Environment(Environment* inOuter);

		Environment* find(const string& var);

		cell_t* get(const string& var);

		cell_t* eval(cell_t* x); 
	};

	struct bool_cell : public cell_t {
		bool value;
		bool_cell(bool inValue) :cell_t(kCellType_bool), value(inValue) {}
		virtual operator string() { return value ? "#t" : "#f"; }
	};

	struct number_cell : public cell_t {
		string valueString;
		double value;
		number_cell(double inValue) :cell_t(kCellType_number), value(inValue) {}
		virtual operator string(); 
	};

	struct char_cell : public cell_t { 
		char value;
		char_cell(char inValue) :cell_t(kCellType_char), value(inValue) {}
		virtual operator string() { std::ostringstream ss; ss << value; return ss.str(); }
	};

	struct string_cell : public cell_t {
		string value;
		string_cell(string inVal) :cell_t(kCellType_string), value(inVal) {}
		virtual operator string() { return value; }
	};

	struct symbol_cell : public cell_t {
		string identifier;
		symbol_cell(string id) :cell_t(kCellType_symbol), identifier(id) {}
		virtual operator string() { return identifier; }
	};

	struct cons_cell : public cell_t {
		cell_t* car;
		cons_cell* cdr;

		cons_cell(cell_t* inCar, cons_cell* inCdr);
		virtual operator string();
	};

	typedef cons_cell list_cell; // A list is a singly linked list of cons cells
	list_cell* empty_list = NULL;

	struct proc_cell : public cell_t {
		virtual cell_t* evalProc(list_cell* args, Environment& env) = 0;

		virtual operator string() { return "#procedure"; }

	protected:
		void verifyCell(cons_cell* inCell, string methodName);

		proc_cell() :cell_t(kCellType_procedure) {}
	};


	/**
	 * A wrapper around std::istream that allows you to grab individual tokens lazily.
	 */
	class TokenStream {
		static const std::regex reg;
		std::istream& is;
		std::string line;
	public:
		TokenStream(std::istream& stream);

		/** Gets the next token, or returns the empty string to indicate EOF */
		std::string nextToken();
	};

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

	class Program {
		Environment global_env;

		public:
		Program();

		/// Eval a string of code and give the result as a string.
		string runCode(string inCode);

		/// Given a stream, read and eval the code read from the stream.
		string runCode(TokenStream& stream);

		/// Read eval print loop.
		void repl(string prompt = "elisp> ");
	};
}

#include "internal/elispImpl.h"

