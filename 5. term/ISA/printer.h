/*
 * Autor: Ondrej Vales, xvales03
 * Predmet: ISA 2016/2017
 * Projekt: Monitorovani DHCP komunikace
 */

#ifndef PRINTER_CLASS_FILE
#define PRINTER_CLASS_FILE

#include <string>
#include <iostream>
#include <fstream>
#include <mutex>

#include "network.h"
#include "error.h"

using namespace std;

void printer(vector<network> &networks, int freq);
void printer_to_file(vector<network> &networks, int per);
void print_head();

#endif
