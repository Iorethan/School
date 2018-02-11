all: webclient

webclient: webclient.o
	g++ webclient.o -o webclient
	
webclient.o: webclient.cpp
	g++ -c webclient.cpp

clean:
	rm -f *.o *.html *.core webclient