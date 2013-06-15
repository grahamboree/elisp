/*
 *
 */

#pragma once

////////////////////////////////////////////////////////////////////////////////
class Environment {
public:
	Environment* outer;
	map<string, cell_t*> mSymbolMap;

	Environment() :outer(NULL) {}
	Environment(Environment* inOuter) :outer(inOuter) {}

	Environment* find(string& var) {
		if (mSymbolMap.find(var) != mSymbolMap.end())
			return this;
		trueOrDie(outer != NULL, "Undefined symbol " + var);
		return outer->find(var);
	}

	cell_t* get(string& var) {
		map<string, cell_t*>::iterator position = mSymbolMap.find(var);
		return position->second;
	}

	cell_t* eval(cell_t* x) {
		switch(x->type) {
		case kCellType_symbol: {
			// Symbol lookup in the current environment.
			string& id = static_cast<symbol_cell*>(x)->identifier;
			return find(id)->get(id);
		} break;
		case kCellType_cons: {
			// Function call
			list_cell* listcell = static_cast<list_cell*>(x);

			cell_t* callable = this->eval(listcell->car);

			// If the first argument is a symbol, look it up in the current environment.
			if (callable->type == kCellType_symbol) {
				string callableName = static_cast<symbol_cell*>(callable)->identifier;

				Environment* enclosingEnvironment = find(callableName);
				trueOrDie(enclosingEnvironment, "Undefined function: " + callableName);

				callable = enclosingEnvironment->get(callableName);
			}

			if (callable->type == kCellType_procedure) { // Eval the procedure with the rest of the arguments.
				return static_cast<proc_cell*>(callable)->evalProc(listcell, this);
			} else if (callable->type == kCellType_lambda) { // Eval the lambda with the rest of the arguments.
				return static_cast<lambda_cell*>(callable)->eval(listcell, this);
			} else {
				die("Expected procedure or lambda");
			}
		} break;
		default:{
			// Constant literal
			return x;
		} break;
	}

	return NULL;
	}

};
