CFLAGS=-O3 -Wall
LDFLAGS=-I$(INCLUDE_PATH) -L$(LIBRARY_PATH) -lxapian-1.3

all: snippettest evalgen paramvariations

snippettest: snippettest.cc
	g++ snippettest.cc $(LDFLAGS) $(CFLAGS) -o snippettest

evalgen: evalgen.cc
	g++ evalgen.cc $(LDFLAGS) $(CFLAGS) -o evalgen

paramvariations: paramvariations.cc
	g++ paramvariations.cc $(LDFLAGS) $(CFLAGS) -o paramvariations

clean:
	rm -f snippettest evalgen paramvariations
