/*
 * Autor: Ondrej Vales, xvales03
 * Predmet: ISA 2016/2017
 * Projekt: Monitorovani DHCP komunikace
 */

#include "listener.h"
#include "network.h"
#include "error.h"
#include "message.h"
#include "printer.h"

bool parse_args(int argc, char **argv, vector<network> &networks, int &sec);

mutex networks_acces;

int main (int argc, char **argv)
{
	vector<network> networks;		//struktura obsahujici jednotlive sledovane site
	int sec = 1;					//perioda vypisu
	bool to_file = parse_args(argc, argv, networks, sec);		//zpracovani argumentu

	if (!to_file)		//vypisy na cout
	{
		thread t (printer, ref(networks), sec);
		t.detach();
	}
	else				//vypisy do log.csv (prepinac -c)
	{
		thread t (printer_to_file, ref(networks), sec);
		t.detach();
	}

	message mes;					//DHCP zprava
	listener dhcp_socket;			//objekt se socketem zachycujicim broadcast

	while (true)
	{	
		dhcp_socket.listen(mes);
		mes.parse(networks);			//zpracovani obsahu zpravy
	}
}

bool parse_args(int argc, char **argv, vector<network> &networks, int &sec)
{
	string help = "-h";
	string help_file = "-c";

	bool to_file = false;
	int aux = 1;

	if (argc < 2)		//musi byt zadana alespon jedna sit
	{
		print_error(ERR_ARGS);		
	}

	if (argc == 2 && help.compare(argv[1]) == 0)		//napoveda
	{
		cout << "Help." << endl;
		exit(0);
	}

	if (argc > 3 && help_file.compare(argv[1]) == 0)	//vypis do souboru
	{
		size_t pos;
		try
		{
			sec = stoi(argv[2], &pos);
		}
		catch (const invalid_argument& ia)
		{
			print_error(ERR_ARGS);
		}
		to_file = true;
		aux = 3;
		if (argv[2][pos] != 0 || sec < 1)		//zaporny cas
		{
			print_error(ERR_ARGS);
		}
	}

	for (int i = aux; i < argc; i++)		//zpracovani nazvu siti
	{
		network aux (argv[i]);
		networks.push_back (aux);
	}
	return to_file;
}
