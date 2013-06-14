#include "elisp.h"

#ifdef ELISP_TEST
#	include <gtest/gtest.h>
#	include "test.h"
#endif

int main(int argc, char *argv[]) {
#ifdef ELISP_TEST
	::testing::InitGoogleTest(&argc, argv);
	int retval = RUN_ALL_TESTS();
	return retval;
#else

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
#endif
}
