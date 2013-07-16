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

	struct cell_t;
	class Environment;
	class TokenStream;

	typedef shared_ptr<cell_t> Cell;
	typedef shared_ptr<Environment> Env;

	/*--------------------------------------------------------------------------------*/
	/**
	 * Program
	 */
	class Program {
	public:
		Program();

		/// Eval a string of code and give the result as a string.
		string runCode(string inCode);

		/// Given a stream, read and eval the code read from the stream.
		string runCode(TokenStream& stream);

		/// Read eval print loop.
		void repl(string prompt = "elisp> ");

		static Cell 		atom(const std::string& token);
		static vector<Cell> read(TokenStream& stream);
		static vector<Cell> read(string s);
		static string 		to_string(Cell exp);

	private:
		Env global_env;
	};

	/*--------------------------------------------------------------------------------*/
	/**
	 * Environment
	 */
	class Environment : public std::enable_shared_from_this<Environment> {
	public:
		Environment();
		Environment(Env inOuter);

		Env find(const string& var);
		Cell get(const string& var);
		Cell eval(Cell x); 

	private:
		Env outer;

	public: // TODO This shouldn't be public.
		std::map<string, Cell> mSymbolMap;
	};
	
	/*--------------------------------------------------------------------------------*/
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
		eCellType type; // TODO this shouldn't be public.
		virtual ~cell_t() {}
		
		/// Used by the writer to print the string representation of code.
		virtual operator string() = 0;
	protected:
		cell_t(eCellType inType);
	};

	struct bool_cell : public cell_t {
		template<typename T>
		bool_cell(T inValue);
		bool_cell(bool inValue);
		virtual ~bool_cell() {}

		virtual operator string();
	//private:
		bool value; // TODO this shouldn't be public.
	};

	struct number_cell : public cell_t {
		number_cell(double inValue);
		virtual ~number_cell() {}
		virtual operator string(); 
	//private:
		double value; // TODO this shouldn't be public.
		string valueString; // TODO this shouldn't be public.
	};

	struct char_cell : public cell_t { 
		char_cell(char inValue);
		virtual ~char_cell() {}
		virtual operator string();
	//private:
		char value; // TODO this shouldn't be public.
	};

	struct string_cell : public cell_t {
		string_cell(string inVal) :cell_t(kCellType_string), value(inVal) {}
		virtual ~string_cell() {}
		virtual operator string() { return value; }
	//private:
		string value; // TODO this shouldn't be public.
	};

	struct symbol_cell : public cell_t {
		symbol_cell(string id) :cell_t(kCellType_symbol), identifier(id) {}
		virtual ~symbol_cell() {}
		virtual operator string() { return identifier; }
	//private:
		string identifier; // TODO this shouldn't be public.
	};

	struct cons_cell : public cell_t, public std::enable_shared_from_this<cons_cell> {
		cons_cell(Cell inCar, shared_ptr<cons_cell> inCdr);
		virtual ~cons_cell() {}
		virtual operator string();
	//private:
		Cell car; // TODO this shouldn't be public.
		shared_ptr<cons_cell> cdr; // TODO this shouldn't be public and shouldn't be restricted to cons_cell's.

	};
	shared_ptr<cons_cell> empty_list = nullptr;

	/**
	 * Wrapper for a built-in procedure not expressed in the langauge.
	 */
	struct proc_cell : public cell_t {
		proc_cell(std::function<Cell(shared_ptr<cons_cell>, Env)> procedure);
		virtual ~proc_cell() {}
		virtual operator string();
		Cell evalProc(shared_ptr<cons_cell> args, Env env);
	protected:
		std::function<Cell(shared_ptr<cons_cell>, Env)> mProcedure;
	};

	/**
	 * Structure of a function defined in the langauge.
	 */
	struct lambda_cell : public cell_t {
		lambda_cell(Env outerEnv);
		virtual ~lambda_cell() {}

		virtual Cell eval(shared_ptr<cons_cell> args, Env currentEnv);
		virtual operator string();

	//protected:
		Env env; // TODO this shouldn't be public.
		vector<shared_ptr<symbol_cell>> mParameters; // 0 or more arguments. TODO this shouldn't be public.
		vector<Cell> 					mBodyExpressions; // 1 or more body statements. TODO this shouldn't be public. 
		shared_ptr<symbol_cell> 		mVarargsName = nullptr; // TODO this shouldn't be public. 
	};

	/*--------------------------------------------------------------------------------*/
	// Utility methods
	
	// Assertion functions
	void die(string message) { throw std::logic_error(message); }
	template<typename T> void trueOrDie(T condition, string message) { if (!condition) die(message); }

	/**
	 * Converts a Cell to a bool.  Handles nullptr which is used to
	 * indicate an empty list.
	 */
	bool cell_to_bool(Cell cell);

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
	 * cons_cell* cons_cell = makeList({ new symbol_cell("+"), new number_cell(1), new number_cell(2)});
	 */
	shared_ptr<cons_cell> makeList(std::vector<Cell> list);

	/*--------------------------------------------------------------------------------*/
	/**
	 * TokenStream
	 * A wrapper around std::istream that allows you to grab individual tokens lazily.
	 */
	class TokenStream {
	public:
		TokenStream(std::istream& stream);

		/** Gets the next token, or returns the empty string to indicate EOF */
		std::string nextToken();
	private:
		static const std::regex reg;
		std::istream& is;
		std::string line;
	};
}

#include "internal/elispImpl.h"

