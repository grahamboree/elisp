/*
 *
 */

#pragma once

/**
 * Replaces all occurances of \p from with \p to in \p str
 */
void replaceAll(string& str, const string& from, const string& to) {
	string::size_type pos = 0;
	while((pos = str.find(from, pos)) != string::npos) {
		str.replace(pos, from.length(), to);
		pos += to.length();
	}
}

/**
 * Returns \c true if \p inValue is a string representation of a number, \c false otherwise.
 */
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

/**
 * A helper that creates a lisp list given a vector of the list's contents.
 *
 * A convenient use-case:
 * cons_cell* list_cell = makeList({ new symbol_cell("+"), new number_cell(1), new number_cell(2)});
 */
cons_cell* makeList(std::vector<cell_t*> list) {
	cons_cell* result = nullptr;
	typedef vector<cell_t*>::const_reverse_iterator cri;
	for (cri iter = list.rbegin(); iter != list.rend(); ++iter)
		result = new cons_cell(*iter, result);
	return result;
}
