/*
 *
 */

#pragma once 

class Environment;

////////////////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////////////////
struct cell_t {
	eCellType type;
	
	virtual operator string() = 0;
	operator bool();
protected:
	cell_t(eCellType inType) : type(inType) {}
};

////////////////////////////////////////////////////////////////////////////////
ostream& operator << (ostream& os, cell_t* obj) {
	os << ((string)(*obj));
	return cout;
}

////////////////////////////////////////////////////////////////////////////////
ostream& operator << (ostream& os, cell_t& obj) {
	os << (string)obj;
	return cout;
}

////////////////////////////////////////////////////////////////////////////////
struct bool_cell : public cell_t {
	bool value;
	bool_cell(bool inValue) :cell_t(kCellType_bool), value(inValue) {}
	virtual operator string() { return value ? "#t" : "#f"; }
};

////////////////////////////////////////////////////////////////////////////////
cell_t::operator bool() {
	return (type != kCellType_bool || static_cast<bool_cell*>(this)->value);
}

////////////////////////////////////////////////////////////////////////////////
struct number_cell : public cell_t {
	string valueString;
	double value;
	number_cell(double inValue) :cell_t(kCellType_number), value(inValue) {}
	//virtual operator string() { ostringstream ss; ss << ((value == (int)value) ? (int)value : value); return ss.str(); }
	virtual operator string() {
		if (valueString.empty()) {
			ostringstream ss;
			ss << ((value == (int)value) ? (int)value : value);
			return ss.str();
		}
		return valueString;
	}
};

////////////////////////////////////////////////////////////////////////////////
// Character
struct char_cell : public cell_t { 
	char value;
	char_cell(char inValue) :cell_t(kCellType_char), value(inValue) {}
	virtual operator string() { ostringstream ss; ss << value; return ss.str(); }
};

////////////////////////////////////////////////////////////////////////////////
// Strings
struct string_cell : public cell_t {
	string value;
	string_cell(string inVal) :cell_t(kCellType_string), value(inVal) {}
	virtual operator string() { return value; }
};

////////////////////////////////////////////////////////////////////////////////
// Symbols
struct symbol_cell : public cell_t {
	string identifier;
	symbol_cell(string id) :cell_t(kCellType_symbol), identifier(id) {}
	virtual operator string() { return identifier; }
};

////////////////////////////////////////////////////////////////////////////////
// Pairs & Lists
struct cons_cell : public cell_t {
	cell_t* car;
	cons_cell* cdr;

	cons_cell(cell_t* inCar, cons_cell* inCdr);
	virtual operator string();
	/*
	class iterator {
		cons_cell* position;

		iterator(cons_cell* inPosition) :position(inPosition) {}
	};*/
};

////////////////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////////////////
// Procedures
struct proc_cell : public cell_t {
	virtual cell_t* evalProc(list_cell* args, Environment& env) = 0;

	virtual operator string() {
		return "#procedure";
	}

	void verifyCell(cons_cell* inCell, string methodName) {
		trueOrDie(inCell, "Insufficient arguments provided to " + methodName + ".");
	}
protected:
	proc_cell() :cell_t(kCellType_procedure) {}
};

////////////////////////////////////////////////////////////////////////////////
// Lambdas
struct lambda_cell : public cell_t {
	lambda_cell() :cell_t(kCellType_lambda) {}

	virtual cell_t* eval(list_cell* args, Environment& env);
	virtual operator string() {
		ostringstream ss;
		ss << "(lambda (";

		// parameters
		vector<symbol_cell*>::const_iterator parameterIter = mParameters.begin();
		vector<symbol_cell*>::const_iterator parametersEnd = mParameters.end();
		while (parameterIter != parametersEnd) {
			ss << (*parameterIter);
			++parameterIter;

			if (parameterIter == parametersEnd)
				break;

			ss << " ";
		}
		ss << ") ";


		// body expressions
		vector<cell_t*>::const_iterator bodyExprIter = mBodyExpressions.begin();
		vector<cell_t*>::const_iterator bodyExprsEnd = mBodyExpressions.end();
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

	vector<symbol_cell*> 	mParameters; // list of 0 or more arguments
	vector<cell_t*> 	mBodyExpressions; // list of 1 or more body statements.
};

////////////////////////////////////////////////////////////////////////////////
cons_cell::cons_cell(cell_t* inCar, cons_cell* inCdr)
: cell_t(kCellType_cons)
, car(inCar)
, cdr(inCdr)
{
}

cons_cell::operator string() {
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
