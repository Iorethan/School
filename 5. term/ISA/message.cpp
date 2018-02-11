/*
 * Autor: Ondrej Vales, xvales03
 * Predmet: ISA 2016/2017
 * Projekt: Monitorovani DHCP komunikace
 */

#include "message.h"

extern mutex networks_acces;

message::message()
{
	start = chrono::high_resolution_clock::now();		//doba zacatku programu
}

void message::copy(char *buf, ssize_t size)				//zkopirovani DHCP zpravy ze socketu do zpravy
{
	len = size;
	memset(buffer, 0, BUFFER_SIZE);
	memcpy(buffer, buf, size);
}

void message::parse(vector<network> &networks)			//zpracovani zpravy
{
	string ip;
	int type = -1, lease = 0, pos = 240;				//DHCP options zacinaji od bytu 240

	this->parse_opt(type, lease, pos);					//zpracovani options

	if (pos == len)										//zprava neskoncila option END
	{
		return;
	}

	if (type == 5 && lease > 0)							//DCHP ACK
	{
		for (int i = 16; i < 20; i++)					//zjisteni pridelene IP
		{
			ip += to_string((int)(unsigned char)buffer[i]) + ".";
		}
		ip.pop_back();

		networks_acces.lock();							//pokus o vlozeni IP do vsech network
		for (auto& i : networks)
		{
			double time = this->time_from_start();
			i.add(ip, time, lease);
		}
		networks_acces.unlock();
	}

	if (type == 7)										//DHCP RELEASE
	{
		for (int i = 12; i < 16; i++)					//zjisteni uvolnovane IP
		{
			ip += to_string((int)(unsigned char)buffer[i]) + ".";
		}
		ip.pop_back();

		networks_acces.lock();
		for (auto& i : networks)						//odstraneni zaznamu
		{
			i.remove(ip);
		}
		networks_acces.unlock();
	}
}

int message::parse_lease(int pos)			//vypocet lease time
{
	int lease = 0;
	lease += (unsigned char)buffer[pos + 2] * 256 * 256 * 256;
	lease += (unsigned char)buffer[pos + 3] * 256 * 256;
	lease += (unsigned char)buffer[pos + 4] * 256;
	lease += (unsigned char)buffer[pos + 5];
	return lease;
}

void message::parse_opt(int &type, int &lease, int &pos)		//zpracovani DHCP options
{
	while (pos < len)
	{
		if ((unsigned char)buffer[pos] == 0)			//option 0 pad
		{
			pos++;
		}
		else if ((unsigned char)buffer[pos] == 255)		//end
		{
			break;
		}
		else if ((unsigned char)buffer[pos] == 53)		//ACK or REL
		{
			type = (unsigned char)buffer[pos + 2];
			pos += 3;
		}
		else if ((unsigned char)buffer[pos] == 51)		//lease time
		{
			lease = this->parse_lease(pos);
			pos += 6;
		}
		else											//ostatni
		{
			pos += 2 + (unsigned char)buffer[pos + 1];	
		}
	}	
}

double message::time_from_start()			//doba prichodu zpravy od zacatku programu (pro vypocet konce lease time)
{
	chrono::high_resolution_clock::time_point now = chrono::high_resolution_clock::now();
	chrono::duration<double> time_span = chrono::duration_cast<chrono::duration<double>>(now - start);
	return time_span.count();
}
