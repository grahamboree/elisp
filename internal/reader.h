/*
 *
 */

#pragma once

/**
 * Given a in input stream, split it into a list of string tokens which
 * can easily be consumed by the reader.
 */
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

/**
 * Given a string token, creates the atom it represents
 */
cell_t* atom(const string& token) {
	if (token[0] == '#') {
		const string::value_type& boolid = token[1];
		bool val = (boolid == 't' || boolid == 'T');
		trueOrDie((val || boolid == 'f' || boolid == 'F') && token.size() == 2, "Unknown identifier " + token);
		return new bool_cell(val);
	} else if (token[0] == '"') {
		return new string_cell(token);
	} else if (isNumber(token)) {
		std::istringstream iss(token);
		double value = 0.0;
		iss >> value;
		number_cell* n = new number_cell(value);
		n->valueString = token;
		return n;
	} else {
		return new symbol_cell(token);
	}
}

/**
 * Returns a list of top-level expressions.
 */
vector<cell_t*> read(std::istream& istream) {
	vector<string> tokens = tokenize(istream);
	auto it = tokens.begin();
	auto end = tokens.end();

	vector<vector<cell_t*>> exprStack; // The current stack of nested list expressions.
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
 * Returns a list of top-level expressions.
 */
vector<cell_t*> read(string s) {
	istringstream iss(s);
	return read(iss);
}

string to_string(cell_t* exp) {
	ostringstream ss;
	if (exp)
		ss << exp;
	else
		ss << "'" << "()";
	return ss.str();
}

