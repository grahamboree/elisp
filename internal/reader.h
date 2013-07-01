/*
 *
 */

#pragma once

vector<string> tokenize(std::istream& is) {
	std::regex reg( R"(\s*)" // skip whitespace
					"("
						",@|" 				// splice unquote
						R"([\('`,\)]|)" 			// parens, quoting symbols
						R"("(?:[\\].|[^\\"])*"|)" // string literals
						";.*|" 				// comments
						R"([^\s('"`,;)]*))" // identifiers
					")");

	vector<string> tokens;
	std::smatch match;
	string line;

	while (std::getline(is, line)) {
		while (!line.empty() && std::regex_search(line, match, reg)) {
			trueOrDie(match.prefix().str() == "",
					"unknown characters: " + match.prefix().str());
			if (match.str(1) != "" and match.str(1).at(0) != ';')
				tokens.push_back(match[1]);
			line = match.suffix().str();
		}
		trueOrDie(line == "", "unknown characters: " + line);
	}

	return tokens;
}

cell_t* atom(string token) {
	if (token[0] == '#') {
		string::value_type& boolid = token[1];
		bool val = (boolid == 't' || boolid == 'T');
		trueOrDie((val || boolid == 'f' || boolid == 'F'), "Unknown identifier " + token);
		return new bool_cell(val);
	} else if (token[0] == '"') {
		throw runtime_error("string literals are not implemented");
	} else if (isNumber(token)) {
		number_cell* n = new number_cell(atof(token.c_str()));
		n->valueString = token;
		return n;
	} else {
		return new symbol_cell(token.c_str());
	}
}

cell_t* read_from(vector<string>& inTokens) {
	trueOrDie(!inTokens.empty(), "Unexpected EOF while reading");

	string token = inTokens.back();
	inTokens.pop_back();

	if (token == "(") {
		if (inTokens.back() == ")")
			return empty_list;

		// Generate a linked list of the elements in the list.
		cons_cell* currentPair = new cons_cell(read_from(inTokens), NULL);
		trueOrDie(currentPair->car != NULL,
				"Missing procedure.  Original code was most likely (), which is illegal.");
		list_cell* listatom = currentPair;
		while (inTokens.back() != ")") {
			currentPair->cdr = new cons_cell(read_from(inTokens), NULL);
			currentPair = currentPair->cdr;
		}

		inTokens.pop_back();

		return listatom;
	}
	trueOrDie(token != ")", "Unexpected \")\"");
	return atom(token);
}

vector<cell_t*> read_ahead(const vector<string>& tokens) {
	auto it = tokens.begin();
	auto end = tokens.end();

	vector<vector<cell_t*>> exprStack;
	exprStack.emplace_back(); // top-level scope

	for (;it != end; ++it) {
		const string& token = *it;
		if (token == "(") {
			exprStack.emplace_back();
		} else if (token == ")") {
			trueOrDie(exprStack.size() > 1, "Unexpected ) while reading");
			cell_t* listexpr = makeList(exprStack.back());
			exprStack.pop_back();
			exprStack.back().push_back(listexpr);
		} else {
			exprStack.back().push_back(atom(token));
		}
	}

	trueOrDie(it == end, "expected EOF");
	trueOrDie(exprStack.size() == 1, "Unexpected EOF while reading");
	return exprStack.back();
}

/**
 * Returns a list of top-level expressions wrapped in an implicit (begin ...)
 */
cell_t* read(string s) {
	istringstream iss(s);
	vector<string> tokens = tokenize(iss);
	vector<string> rtokens(tokens.rbegin(), tokens.rend());
	//return read_from(rtokens);
	vector<cell_t*> beginExpr = { new symbol_cell("begin") };
	vector<cell_t*> topLevelExprs = read_ahead(tokens);
	beginExpr.insert(beginExpr.end(), topLevelExprs.begin(), topLevelExprs.end());
	return makeList(beginExpr);
}

string to_string(cell_t* exp) {
	ostringstream ss;
	if (exp)
		ss << exp;
	else
		ss << "'" << "()";
	return ss.str();
}

