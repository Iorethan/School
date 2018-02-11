/*
 * Autor: Ondrej Vales, xvales03
 * Predmet: ISA 2016/2017
 * Projekt: Monitorovani DHCP komunikace
 */

#ifndef NETWORK_CLASS_FILE
#define NETWORK_CLASS_FILE

#include <string>
#include <vector>
#include <cmath>
#include <map>
#include <thread>
#include <chrono>
#include <mutex>
#include <algorithm>
#include <iomanip>
#include <fstream>
#include <istream>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>

using namespace std;

class network
{
	string name;
	int prefix;
	int size;
	int count;
	vector<string> used;
	map<string, double> set;
	vector<thread> threads;

public:
	network(string name);
	network(const network &old);
	bool is_in(string ip);
	void add(string ip, double time, int lease);
	void remove(string ip);
	void watcher(string ip, double time, int lease);
	void print();
	void print_to_file(ofstream &file);
	
	static void to_char(char *buf, string ip);
	static void mask(char *buf, int mask);
	static bool is_reserved(char *buf, int mask);
};

#endif
