/*
 *
 */

#pragma once

/**
 * A wrapper around std::istream that allows you to grab individual tokens lazily.
 */
class TokenStream {
	static const std::regex reg;
	std::istream& is;
	std::string line;
public:
TokenStream(std::istream& stream) : is(stream) {}

	/** Gets the next token, or returns the empty string to indicate EOF */
	std::string nextToken() {
		if (line.empty() and !std::getline(is, line))
			return "";

		std::smatch match;
		if (std::regex_search(line, match, reg)) {
			trueOrDie(match.prefix().str() == "", "unknown characters: " + match.prefix().str());

			std::string matchStr = match[1].str();
			line = match.suffix().str();
			if (matchStr.empty() or matchStr[0] == ';')
				return nextToken();
			else
				return matchStr;
				
		} else {
			trueOrDie(false, "Unknown characters: " + line);
		}
		return "";
	}
};

/** Tokenizer regex */
const std::regex TokenStream::reg(
		R"(\s*)" // skip whitespace
	   "("
			",@|" 				// splice unquote
			R"([\('`,\)]|)" 			// parens, quoting symbols
			R"("(?:[\\].|[^\\"])*"|)" // string literals
			";.*|" 				// comments
			R"([^\s('"`,;)]*))" // identifiers
		")");

/**
 * Given a string token, creates the atom it represents
 */
cell_t* atom(const std::string& token) {
	if (token[0] == '#') {
		const std::string::value_type& boolid = token[1];
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
std::vector<cell_t*> read(TokenStream& stream) {
	std::vector<std::vector<cell_t*>> exprStack; // The current stack of nested list expressions.
	exprStack.emplace_back(); // top-level scope

	for (std::string token = stream.nextToken(); !token.empty(); token = stream.nextToken()) {
		if (token == "(") {
			// Push a new scope onto the stack.
			exprStack.emplace_back();
		} else if (token == ")") {
			// Pop the current scope off the stack and add it as a list to its parent scope.
			trueOrDie(exprStack.size() > 1, "Unexpected ) while reading");
			cell_t* listexpr = makeList(exprStack.back());
			exprStack.pop_back();
			exprStack.back().push_back(listexpr);
		} else {
			exprStack.back().push_back(atom(token));
		}
	}

	trueOrDie(exprStack.size() == 1, "Unexpected EOF while reading");
	return exprStack.back();
}

/**
 * Returns a list of top-level expressions.
 */
std::vector<cell_t*> read(string s) {
	std::istringstream iss(s);
	TokenStream tokStream(iss);
	return read(tokStream);
}

std::string to_string(cell_t* exp) {
	std::ostringstream ss;
	if (exp)
		ss << exp;
	else
		ss << "'()";
	return ss.str();
}

