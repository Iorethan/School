/*
 * Autor: Ondrej Vales, xvales03
 * Predmet: ISA 2016/2017
 * Projekt: Monitorovani DHCP komunikace
 */

#include "error.h"

void print_error(int error_number)
{
	vector<string> message = 
	{
		"OK, why was called error?",
		"ERROR, unspecified problem",
		"ERROR, bad arguments",
		"ERROR, unable to create socket (sudo?)",
		"ERROR, unable to recieve datagram"
	};

	cerr << message[error_number] << endl;		//vypis chyboveho hlaseni
	exit(error_number);
}
