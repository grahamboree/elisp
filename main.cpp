
#include <gtest/gtest.h>
#include "elisp.h"

#include "test.h"

int main(int argc, char *argv[]) {
	::testing::InitGoogleTest(&argc, argv);
	int retval = RUN_ALL_TESTS();

	if (argc > 1) {
		// If -t was specified, run tests.

		// File(s) were specified, so run those
		ifstream t(argv[1]);

		ostringstream ss;
		string line;
		while (getline(t, line))
			ss << removeComments(line);

		runCode(ss.str());
	} else {
		repl();
	}

	return retval;
}
