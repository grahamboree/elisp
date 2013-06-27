/*
 *
 */

#ifdef ELISP_TEST
#	define CATCH_CONFIG_MAIN
#	include "catch.hpp"  // The catch unit testing framework
#endif 

#include "elisp.h"    // The entire elisp library

#ifdef ELISP_TEST
#	include "internal/test.h" // Norvig's lispy test suite
#else
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
			ss << Program::removeComments(line);

		p.runCode(ss.str());
	} else {
		p.repl();
	}
	return 0;
}
#endif
