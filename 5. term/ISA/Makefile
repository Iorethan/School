
# Autor: Ondrej Vales, xvales03
# Predmet: ISA 2016/2017
# Projekt: Monitorovani DHCP komunikace

CXX = g++
CXXFLAGS = -Wall -Wextra -pedantic -std=c++11
OFILES = dhcp-stats.o error.o listener.o network.o message.o printer.o
HFILES = buffer.h error.h listener.h network.h message.h printer.h

all: dhcp-stats

dhcp-stats: $(OFILES) $(HFILES)
	$(CXX) $(CXXFLAGS) $(OFILES) -lpthread -o dhcp-stats
	
dhcp-stats.o: dhcp-stats.cpp
	$(CXX) $(CXXFLAGS) -c dhcp-stats.cpp

error.o: error.cpp error.h
	$(CXX) $(CXXFLAGS) -c error.cpp

listener.o: listener.cpp listener.h buffer.h
	$(CXX) $(CXXFLAGS) -c listener.cpp

network.o: network.cpp network.h
	$(CXX) $(CXXFLAGS) -c network.cpp

message.o: message.cpp message.h buffer.h
	$(CXX) $(CXXFLAGS) -c message.cpp

printer.o: printer.cpp printer.h
	$(CXX) $(CXXFLAGS) -c printer.cpp

clean:
	rm -f *.o *.csv dhcp-stats