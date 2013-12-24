COMPILE = clang++ -Wall -Wextra -pedantic -std=c++11 -stdlib=libc++
INCLUDES = -I./src/ -I./src/interal -I./lib/
DEFINES =
EXEC_NAME = elisp

HEADERS = $(shell ls ./src/*.h ./src/internal/*.h)
ELISP_SRC = main.cpp

all: elisp lispy

lispy:
	python ./lispy_test.py

test: $(HEADERS) elisp.pch
	$(COMPILE) $(INCLUDES) -O0 -DCATCH_CONFIG_MAIN -DELISP_TEST -include catch.hpp $(ELISP_SRC) -o elispTests
	./elispTests

#elisp: test $(HEADERS) 
elisp: $(HEADERS) $(ELISP_SRC)
	$(COMPILE) $(INCLUDES) $(DEFINES) $(ELISP_SRC) -O4 -o $(EXEC_NAME)

elisp.pch: ./lib/catch.hpp
	$(COMPILE) -DCATCH_CONFIG_MAIN -x c++-header ./lib/catch.hpp -o elisp.pch

clean:
	rm -f *.o
	rm -f *~
	rm -f elisp
	rm -f elispTests
	rm -f elisp.pch
