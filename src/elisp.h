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

	class cell_t;
	class Environment;
	class TokenStream;

	typedef shared_ptr<cell_t> Cell;
	typedef shared_ptr<Environment> Env;

	/*--------------------------------------------------------------------------------*/
	/** Program
	 * Represents a discrete execution environment for elisp.
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

	/*--------------------------------------------------------------------------------*/
	/** Environment
	 * A symbol mapping environment.  Designed as a tree where each node
	 * represents a scope which can override the parent scope bindings.
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
	// Cells
	
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
	class cell_t {
	public:
		virtual ~cell_t() {}
		
		/// Used by the writer to print the string representation of code.
		virtual operator string() = 0;

		eCellType GetType() { return type; }
	protected:
		cell_t(eCellType inType);
		eCellType type;
	};

	class bool_cell : public cell_t {
	public:
		template<typename T>
		bool_cell(T inValue);
		bool_cell(bool inValue);
		virtual ~bool_cell() {}

		virtual operator string();

		bool GetValue() { return value; }
	private:
		bool value;
	};

	class number_cell : public cell_t {
	public:
		number_cell(double inValue);
		virtual ~number_cell() {}
		virtual operator string(); 

		double GetValue() { return value; }
		void SetValueString(string valuestr) { valueString = valuestr; }
	private:
		double value;
		string valueString;
	};

	class char_cell : public cell_t { 
	public:
		char_cell(char inValue);
		virtual ~char_cell() {}
		virtual operator string();
	private:
		char value;
	};

	class string_cell : public cell_t {
	public:
		string_cell(string inVal) :cell_t(kCellType_string), value(inVal) {}
		virtual ~string_cell() {}
		virtual operator string() { return value; }
	private:
		string value;
	};

	class symbol_cell : public cell_t {
	public:
		symbol_cell(string id) :cell_t(kCellType_symbol), identifier(id) {}
		virtual ~symbol_cell() {}
		virtual operator string() { return identifier; }

		string GetIdentifier() { return identifier; }
	private:
		string identifier;
	};

	class cons_cell : public cell_t, public std::enable_shared_from_this<cons_cell> {
	public:
		class iterator {
			shared_ptr<cons_cell> currentCell;
		public:
			iterator(shared_ptr<cons_cell> startCell);

			iterator& operator++();
			iterator operator++(int);
			bool operator==(const iterator& other);
			bool operator!=(const iterator& other);

			Cell operator *();
		};

		cons_cell(Cell inCar, Cell inCdr);
		virtual ~cons_cell() {}
		virtual operator string();

		Cell GetCar();
		void SetCar(Cell newCar);

		Cell GetCdr();
		void SetCdr(Cell newCdr);

		iterator begin();
		iterator end();

	private:
		Cell car;
		Cell cdr;
	};
	shared_ptr<cons_cell> empty_list = nullptr;

	/**
	 * Wrapper for a built-in procedure not expressed in the langauge.
	 */
	class proc_cell : public cell_t {
	public:
		proc_cell(std::function<Cell(shared_ptr<cons_cell>, Env)> procedure);
		virtual ~proc_cell() {}
		virtual operator string();
		Cell evalProc(shared_ptr<cons_cell> args, Env env);
	private:
		std::function<Cell(shared_ptr<cons_cell>, Env)> mProcedure;
	};

	/**
	 * Structure of a function defined in the langauge.
	 */
	class lambda_cell : public cell_t {
	public:
		lambda_cell(Env outerEnv);
		lambda_cell(Env outerEnv, vector<shared_ptr<symbol_cell>>&& inParameters, vector<Cell>&& inBodyExpressions, shared_ptr<symbol_cell>&& inVarargsName);
		virtual ~lambda_cell() {}

		virtual Cell eval(shared_ptr<cons_cell> args, Env currentEnv);
		virtual operator string();

	private:
		Env env;
		vector<shared_ptr<symbol_cell>> mParameters; // 0 or more arguments. 
		vector<Cell> 					mBodyExpressions; // 1 or more body statements. 
		shared_ptr<symbol_cell> 		mVarargsName; // name of the varargs parameter if there is one.
	};
}

#include "internal/elispImpl.h"
