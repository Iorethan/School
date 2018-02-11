/*
 * Autor: Ondrej Vales, xvales03
 * Predmet: ISA 2016/2017
 * Projekt: Monitorovani DHCP komunikace
 */

#ifndef SOCKET_CLASS__FILE
#define SOCKET_CLASS__FILE

#include <iostream>
#include <vector>
#include <unistd.h>

using namespace std;

enum error_number
{
	OK,
	ERR_GEN,
	ERR_ARGS,
	ERR_SOCK,
	ERR_RECV
};

void print_error(int error_number);

#endif
