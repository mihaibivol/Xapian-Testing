all: snippettest.cc
	g++ snippettest.cc -I$(INCLUDE_PATH) -L$(LIBRARY_PATH) -lxapian-1.3 -o snippettest

clean:
	rm -f snippettest
