#pragma once

#include <algorithm>
#include <iostream>
#include <fstream>
#include <iterator>
#include <list>
#include <map>
#include <memory>
#include <sstream>
#include <streambuf>
#include <string>
#include <vector>

using namespace std;

#include "util.h"

struct Env;

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

struct cell_t {
	eCellType type;
	
	virtual operator string() = 0;
protected:
	cell_t(eCellType inType) : type(inType) {}
};

// Booleans
struct bool_cell : public cell_t {
	bool value;
	bool_cell(bool inValue) :cell_t(kCellType_bool), value(inValue) {}
	virtual operator string() { return value ? "#t" : "#f"; }
};

// Number
struct number_cell : public cell_t {
	double value;
	number_cell(double inValue) :cell_t(kCellType_number), value(inValue) {}
	virtual operator string() { ostringstream ss; ss << value; return ss.str(); }
};

// Character
struct char_cell : public cell_t { 
	char value;
	char_cell(char inValue) :cell_t(kCellType_char), value(inValue) {}
	virtual operator string() { ostringstream ss; ss << value; return ss.str(); }
};

// Strings
struct string_cell : public cell_t {
	string value;
	string_cell(string inVal) :cell_t(kCellType_string), value(inVal) {}
	virtual operator string() { return value; }
};

// Symbols
struct symbol_cell : public cell_t {
	string identifier;
	symbol_cell(string id) :cell_t(kCellType_symbol), identifier(id) {}
	virtual operator string() { return identifier; }
};

// Pairs & Lists
struct cons_cell : public cell_t {
	cell_t* car;
	cons_cell* cdr;

	cons_cell(cell_t* inCar, cons_cell* inCdr) :cell_t(kCellType_cons), car(inCar), cdr(inCdr) {}
	virtual operator string() {
		ostringstream ss;
		ss << "(";

		cons_cell* currentCell = this;
		while (currentCell != NULL) {
			ss << currentCell->car;
			currentCell = currentCell->cdr;
			if (currentCell)
				ss << " ";
		}
		ss << ")";
		return ss.str();
	}

	class iterator {
		cons_cell* position;

		iterator(cons_cell* inPosition) :position(inPosition) {}
	};
};
typedef cons_cell list_cell; // A list is a singly linked list of cons cells
list_cell* empty_list = NULL;

// Vectors
/*
struct vector_cell : public cell_t {
	vector<cell_t*> value;
	vector_cell() :cell_t(kCellType_vector) {}
	vector_cell(vector<cell_t*>& inVal) :cell_t(kCellType_vector), value(inVal) {}
	virtual operator string() {
		ostringstream ss;
		ss << "#(";

		vector<cell_t*>::iterator vectIter = value.begin();
		vector<cell_t*>::iterator vectEnd = value.end();
		for (; vectIter != vectEnd; ++vectIter)
			ss << (*vectIter);
		ss << ")";
		return ss.str();
	}
};*/

// Procedures
struct proc_cell : public cell_t {
	virtual cell_t* evalProc(list_cell* args, Env* env) = 0;

	virtual operator string() {
		return "#procedure";
	}

	void verifyCell(cons_cell* inCell, string methodName) {
		trueOrDie(inCell, "Error, insufficient arguments provided to " + methodName);
	}
protected:
	proc_cell() :cell_t(kCellType_procedure) {}
};

cell_t* eval(cell_t* x, Env* env);

// Lambdas
struct lambda_cell : public cell_t {
	lambda_cell() :cell_t(kCellType_lambda) {}

	virtual cell_t* eval(list_cell* args, Env* env);
	virtual operator string() {
		ostringstream ss;
		ss << "(lambda (";

		// parameters
		list<symbol_cell*>::const_iterator parameterIter = mParameters.begin();
		list<symbol_cell*>::const_iterator parametersEnd = mParameters.end();
		while (parameterIter != parametersEnd) {
			ss << (*parameterIter);
			++parameterIter;

			if (parameterIter == parametersEnd)
				break;

			ss << " ";
		}
		ss << ") ";


		// body expressions
		list<cell_t*>::const_iterator bodyExprIter = mBodyExpressions.begin();
		list<cell_t*>::const_iterator bodyExprsEnd = mBodyExpressions.end();
		while (bodyExprIter != bodyExprsEnd) {
			ss << (*bodyExprIter);
			++bodyExprIter;

			if (bodyExprIter == bodyExprsEnd)
				break;

			ss << " ";
		}
		ss << ")";
		
		return ss.str();
	}

	list<symbol_cell*> 	mParameters; // list of 0 or more arguments
	list<cell_t*> 	mBodyExpressions; // list of 1 or more body statements.
};

struct Env {
	Env* outer;
	map<string, cell_t*> mSymbolMap;

	Env() :outer(NULL) {}
	Env(Env* inOuter) :outer(inOuter) {}

	Env* find(string& var) {
		if (mSymbolMap.find(var) != mSymbolMap.end())
			return this;
		trueOrDie(outer != NULL, "Undefined symbol " + var);
		return outer->find(var);
	}

	cell_t* get(string& var) {
		map<string, cell_t*>::iterator position = mSymbolMap.find(var);
		return position->second;
	}
};

cell_t* lambda_cell::eval(list_cell* args, Env* env) {
	Env* newEnv = new Env(env);

	// Match the arguments to the parameters.
	list<symbol_cell*>::const_iterator parameterIter = mParameters.begin();
	list<symbol_cell*>::const_iterator parametersEnd = mParameters.end();
	for (; parameterIter != parametersEnd; ++parameterIter) {
		args = args->cdr;
		trueOrDie(args != empty_list, "insufficient arguments provided to function");
		newEnv->mSymbolMap[(*parameterIter)->identifier] = args->car;
	}

	// Evaluate the body expressions with the new environment.  Return the result of the last body expression.
	cell_t* returnVal;
	list<cell_t*>::iterator bodyExprIter = mBodyExpressions.begin();
	list<cell_t*>::iterator bodyExprsEnd = mBodyExpressions.end();
	for (; bodyExprIter != bodyExprsEnd; ++bodyExprIter)
		returnVal = ::eval(*bodyExprIter, newEnv);

	return returnVal;
}

bool cell_to_bool(cell_t* inCell) {
	return (inCell->type != kCellType_bool || static_cast<bool_cell*>(inCell)->value);
}

ostream& operator << (ostream& os, cell_t* obj) {
	os << ((string)(*obj));
	return cout;
}

ostream& operator << (ostream& os, cell_t& obj) {
	os << (string)obj;
	return cout;
}

