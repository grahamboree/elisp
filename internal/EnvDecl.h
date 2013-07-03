/*
 *
 *
 */

#pragma once

struct cell_t;

////////////////////////////////////////////////////////////////////////////////
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
