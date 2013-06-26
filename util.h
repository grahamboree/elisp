#pragma once

#include <cstdarg>
#include "Cells.h"
#include "Assert.h"

////////////////////////////////////////////////////////////////////////////////
void printTokenList(list<string> inTokens) {
	list<string>::const_iterator tokenIter = inTokens.begin();
	list<string>::const_iterator tokensEnd = inTokens.end();

	for (;tokenIter != tokensEnd; ++tokenIter, cout << " ")
		cout << *tokenIter;
	cout << endl;
}

////////////////////////////////////////////////////////////////////////////////
void replaceAll(string& str, const string& from, const string& to) {
	string::size_type pos = 0;
	while((pos = str.find(from, pos)) != string::npos) {
		str.replace(pos, from.length(), to);
		pos += to.length();
	}
}

////////////////////////////////////////////////////////////////////////////////
bool isNumber(string inValue) {
	string::const_iterator it = inValue.begin();
	bool hasRadix = false;
	bool hasDigit = false;

	if (!inValue.empty() && ((hasDigit = (isdigit(*it) != 0)) || *it == '-')) {
		++it;
		for (;it != inValue.end(); ++it) {
			bool digit = (isdigit(*it) != 0);
			hasDigit = hasDigit || digit;
			if (!(digit || (!hasRadix && (hasRadix = (*it == '.')))))
				break;
		}
		return hasDigit && it == inValue.end();
	}
	return false;
}

////////////////////////////////////////////////////////////////////////////////
cons_cell* makeList(std::vector<cell_t*> list) {
	cons_cell* result = nullptr;
	typedef vector<cell_t*>::const_reverse_iterator cri;
	for (cri iter = list.rbegin(); iter != list.rend(); ++iter)
		result = new cons_cell(*iter, result);
	return result;
}
