GTEST_DIR = ./gtest
INCLUDES = -I${GTEST_DIR}/include
DEFINES =
COMPILE = clang++ $(INCLUDES) -Wall -Wextra
EXEC_NAME = elisp

ELISP_SRC = main.cpp
GTEST_SRC = ${GTEST_DIR}/src/*.cc

all: elisp

test: DEFINES = -DELISP_TEST
test: EXEC_NAME = elispTests
test: elisp
	./elispTests

elisp: libgtest.a
	$(COMPILE) $(DEFINES) $(ELISP_SRC) ./gtest/lib/libgtest.a -lpthread -o $(EXEC_NAME)

libgtest.a: ${GTEST_SRC}
	clang++ -I${GTEST_DIR}/include -I${GTEST_DIR} -c ${GTEST_DIR}/src/gtest-all.cc
	mkdir -p ${GTEST_DIR}/lib
	ar -rv ${GTEST_DIR}/lib/libgtest.a gtest-all.o

clean:
	rm -f ./*.o
	rm -f *~
	rm -f elisp
	rm -f elispTests
	rm -f a.out
	rm -f ./gtest/gtest-all.o
	rm -f ./gtest/lib/libgtest.a
