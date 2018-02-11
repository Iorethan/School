/*
 * Autor: Ondrej Vales, xvales03
 * Predmet: ISA 2016/2017
 * Projekt: Monitorovani DHCP komunikace
 */

#ifndef MESSAGE_CLASS_FILE
#define MESSAGE_CLASS_FILE

#include <cstring> 
#include <chrono>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sys/types.h>

#include "buffer.h"
#include "network.h"

using namespace std;

class message
{
	char buffer[BUFFER_SIZE];
	ssize_t len;
	chrono::high_resolution_clock::time_point start;

public:
	message();
	void copy(char *buf, ssize_t size);
	void parse(vector<network> &networks);
	int parse_lease(int pos);
	void parse_opt(int &type, int &lease, int &pos);
	double time_from_start();
};

#endif
