/*
 *
 */

#pragma once 

////////////////////////////////////////////////////////////////////////////////
cell_t* lambda_cell::eval(list_cell* args) {
	Environment* newEnv = new Environment(env);

	// Match the arguments to the parameters.
	vector<symbol_cell*>::const_iterator parameterIter = mParameters.begin();
	vector<symbol_cell*>::const_iterator parametersEnd = mParameters.end();
	for (; parameterIter != parametersEnd; ++parameterIter) {
		trueOrDie(args != empty_list, "insufficient arguments provided to function");
		newEnv->mSymbolMap[(*parameterIter)->identifier] = env->eval(args->car);
		args = args->cdr;
	}

	// Evaluate the body expressions with the new environment.  Return the result of the last body expression.
	cell_t* returnVal;
	vector<cell_t*>::iterator bodyExprIter = mBodyExpressions.begin();
	vector<cell_t*>::iterator bodyExprsEnd = mBodyExpressions.end();
	for (; bodyExprIter != bodyExprsEnd; ++bodyExprIter)
		returnVal = newEnv->eval(*bodyExprIter);

	return returnVal;
}

