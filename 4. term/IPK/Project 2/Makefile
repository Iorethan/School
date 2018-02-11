all: server client

server: server.o server.cpp
	g++ -std=c++0x server.o -o server -static-libstdc++ -lpthread
	
server.o: server.cpp
	g++ -std=c++0x -c server.cpp

client: client.o client.cpp
	g++ -std=c++0x client.o -o client -static-libstdc++
	
client.o: client.cpp
	g++ -std=c++0x -c client.cpp
	
pack:
	tar pczf xvales03.tar.gz Makefile server.cpp client.cpp dokumentace.pdf test.sh test.txt ref_test.txt README.md

clean:
	rm -f *.o *.core server client *tar.gz