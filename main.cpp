/*
 *
 */

#ifdef ELISP_TEST

#	define CATCH_CONFIG_MAIN
#	include "catch.hpp" // The catch unit testing framework
#	include "elisp.h" 	// The entire elisp library
#	include "internal/test.h" // Norvig's lispy test suite

#else 

#include <fstream>
#include "elisp.h" // The entire elisp library
using namespace elisp;
using namespace std;

// If given a file argument, runs the file,
// otherwise it just runs the repl
int main(int argc, char *argv[]) {
	Program p;
	if (argc > 1) {
		// File(s) were specified, so run those
		ifstream t(argv[1]);

		ostringstream ss;
		string line;
		while (getline(t, line))
			ss << removeComments(line);

		p.runCode(ss.str());
	} else {
		p.repl();
	}
	return 0;
}

#endif
