all: snippettest evalgen

snippettest: snippettest.cc
	g++ snippettest.cc -I$(INCLUDE_PATH) -L$(LIBRARY_PATH) -lxapian-1.3 -o snippettest

evalgen: evalgen.cc
	g++ evalgen.cc -I$(INCLUDE_PATH) -L$(LIBRARY_PATH) -lxapian-1.3 -o evalgen

clean:
	rm -f snippettest
