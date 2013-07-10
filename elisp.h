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
	using std::shared_ptr;

	// Assertion functions
	void die(string message) { throw std::logic_error(message); }
	template<typename T> void trueOrDie(T condition, string message) { if (!condition) die(message); }

	struct cell_t;
	class Environment;

	////////////////////////////////////////////////////////////////////////////////
	// Environment
	typedef std::shared_ptr<Environment> Env;
	class Environment : public std::enable_shared_from_this<Environment> {
	public:
		Environment();
		Environment(Env inOuter);

		Env find(const string& var);
		cell_t* get(const string& var);
		cell_t* eval(cell_t* x); 

	private:
		Env outer;

	public:
		std::map<string, cell_t*> mSymbolMap;
	};
	
	////////////////////////////////////////////////////////////////////////////////
	// Cell types
	/// Data types in elisp
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
		virtual cell_t* evalProc(list_cell* args, Env env) = 0;
		virtual operator string() { return "#procedure"; }
	protected:
		void verifyCell(cons_cell* inCell, string methodName);

		proc_cell() :cell_t(kCellType_procedure) {}
	};

	struct lambda_cell : public cell_t {
		Env env;
		lambda_cell(Env outerEnv) :cell_t(kCellType_lambda), env(outerEnv) {}

		virtual cell_t* eval(list_cell* args, Env currentEnv);
		virtual operator string();

		vector<symbol_cell*> 	mParameters; // list of 0 or more arguments
		vector<cell_t*> 	mBodyExpressions; // list of 1 or more body statements.
	};

	////////////////////////////////////////////////////////////////////////////////
	// Util.h
	bool cell_to_bool(cell_t* cell);

	/**
	 * Replaces all occurances of \p from with \p to in \p str
	 */
	void replaceAll(string& str, const string& from, const string& to);

	/**
	 * Returns \c true if \p inValue is a string representation of a number, \c false otherwise.
	 */
	bool isNumber(string inValue);

	/**
	 * A helper that creates a lisp list given a vector of the list's contents.
	 *
	 * A convenient use-case:
	 * cons_cell* list_cell = makeList({ new symbol_cell("+"), new number_cell(1), new number_cell(2)});
	 */
	cons_cell* makeList(std::vector<cell_t*> list);


	////////////////////////////////////////////////////////////////////////////////
	// TokenStream
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

	////////////////////////////////////////////////////////////////////////////////
	// Program
	class Program {
		Env global_env;

	public:
		Program();

		/// Eval a string of code and give the result as a string.
		string runCode(string inCode);

		/// Given a stream, read and eval the code read from the stream.
		string runCode(TokenStream& stream);

		/// Read eval print loop.
		void repl(string prompt = "elisp> ");

		static auto atom(const std::string& token) 	-> cell_t*;
		static auto read(TokenStream& stream) 		-> std::vector<cell_t*>;
		static auto read(string s) 					-> std::vector<cell_t*>;
		static auto to_string(cell_t* exp) 			-> std::string;
	};
}

#include "internal/elispImpl.h"

