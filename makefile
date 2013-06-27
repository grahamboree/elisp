COMPILE = clang++ -Wall -Wextra -pedantic -std=c++11 -stdlib=libc++ -O4
INCLUDES = 
DEFINES =
EXEC_NAME = elisp

HEADERS = $(shell ls *.h ./internal/*.h)
ELISP_SRC = main.cpp

all: elisp

#test: DEFINES = -DELISP_TEST
#test: EXEC_NAME = elispTests

test: $(HEADERS)
	$(COMPILE) $(INCLUDES) -DELISP_TEST $(ELISP_SRC) -o elispTests
	./elispTests

#elisp: test $(HEADERS) 
elisp: $(HEADERS) 
	$(COMPILE) $(INCLUDES) $(DEFINES) $(ELISP_SRC) -o $(EXEC_NAME)

clean:
	rm -f *.o
	rm -f *~
	rm -f elisp
	rm -f elispTests
