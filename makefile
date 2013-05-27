
GTEST_DIR = ./gtest
INCLUDES = -I${GTEST_DIR}/include -I.
COMPILE = clang++ $(INCLUDES) -Wall -Wextra
LINK = clang++ -o 

ELISP_SRC = main.cpp
GTEST_SRC = ${GTEST_DIR}/src/*.cc


all: elisp
	#./elisp -t

elisp: libgtest.a
	$(COMPILE) $(ELISP_SRC) ./gtest/lib/libgtest.a -lpthread -o elisp

libgtest.a: ${GTEST_SRC}
	clang++ -I${GTEST_DIR}/include -I${GTEST_DIR} -c ${GTEST_DIR}/src/gtest-all.cc
	ar -rv ${GTEST_DIR}/lib/libgtest.a gtest-all.o

clean:
	rm -f ./*.o
	rm -f *~
	rm -f elisp
	rm -f a.out
	rm -f ./gtest/gtest-all.o
	rm -f ./gtest/lib/libgtest.a


#all: gtest elisp
#	#clang++ uscheme.cpp -Wall
#	#clang++ cppmonad.cpp -Wall
#	elisp.cpp
#
#elisp: elisp.o 
#	$(LINK) $@ $^
#
#%.o: %.cpp
#	g++ -c $<
#
#clean:
#	rm -f ./*.o
#	rm -f *~
