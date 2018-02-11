/*
 * Autor: Ondrej Vales, xvales03
 * Predmet: ISA 2016/2017
 * Projekt: Monitorovani DHCP komunikace
 */

#include "printer.h"

extern mutex networks_acces;

void printer(vector<network> &networks, int per)		//vypis na cout
{
	int lines = networks.size() + 1;
	while(true)
	{
		networks_acces.lock();
		print_head();						//vypis hlavicky
		for (auto& i : networks)
		{
			i.print();						//vypis pro kazdou zadanou sit
		}
		networks_acces.unlock();
		this_thread::sleep_for(chrono::seconds(per));		//vypis jednaou za per sekund (implicitne 1)

		for (int i = 0; i < lines; i++)			//posun na puvodni radek (opakovany prepis stejnych radku terminalu)
		{
			cout << "\x1b[A";
		}
	}
}

void printer_to_file(vector<network> &networks, int per)		//vypis do logu
{
	ofstream file ("log.csv", std::ofstream::out);		//otevreni log souboru
	if (!file.is_open())
	{
		print_error(ERR_GEN);
	}
	while(true)
	{
		networks_acces.lock();

		time_t ts = time(nullptr);
		file << ts << ',' << asctime(localtime(&ts));
		file.seekp(-1, ios_base::cur);
		file << ',' << endl;
		
		for (auto& i : networks)
		{
			i.print_to_file(file);					//vypis jednotlivych siti
		}
		file << endl;
		networks_acces.unlock();
		this_thread::sleep_for(chrono::seconds(per));
	}
}

void print_head()
{
	cout << "IP Prefix            Max hosts        Allocated addresses           Utilization" << endl;
}
