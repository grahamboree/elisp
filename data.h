/*
 *
 */

#pragma once 

#include <algorithm>
#include <fstream>
#include <iostream>
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
#include "Cells.h"
#include "Environment.h"

////////////////////////////////////////////////////////////////////////////////
cell_t* lambda_cell::eval(list_cell* args, Environment& env) {
	Environment newEnv(env);

	// Match the arguments to the parameters.
	list<symbol_cell*>::const_iterator parameterIter = mParameters.begin();
	list<symbol_cell*>::const_iterator parametersEnd = mParameters.end();
	for (; parameterIter != parametersEnd; ++parameterIter) {
		trueOrDie(args != empty_list, "insufficient arguments provided to function");
		newEnv.mSymbolMap[(*parameterIter)->identifier] = env.eval(args->car);
		args = args->cdr;
	}

	// Evaluate the body expressions with the new environment.  Return the result of the last body expression.
	cell_t* returnVal;
	list<cell_t*>::iterator bodyExprIter = mBodyExpressions.begin();
	list<cell_t*>::iterator bodyExprsEnd = mBodyExpressions.end();
	for (; bodyExprIter != bodyExprsEnd; ++bodyExprIter)
		returnVal = newEnv.eval(*bodyExprIter);

	return returnVal;
}

