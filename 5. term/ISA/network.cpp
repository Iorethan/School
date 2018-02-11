/*
 * Autor: Ondrej Vales, xvales03
 * Predmet: ISA 2016/2017
 * Projekt: Monitorovani DHCP komunikace
 */

#include "network.h"
#include "error.h"

extern mutex networks_acces;

network::network(string name)
{
	size_t pos = name.find_last_of ('/');		//oddeleni prefixu
	size_t idx = 0;
	if (pos + 1 == name.size() || pos == string::npos)		//chybi oddelovac nebo prefix
	{
		print_error(ERR_ARGS);
	}

	try
	{
		prefix = stoi(name.substr(pos + 1), &idx);		//prevod prefixu
	}
	catch (const invalid_argument& e)
	{
		print_error(ERR_ARGS);
	}

	if (idx + pos + 1 != name.size())
	{
		print_error(ERR_ARGS);
	}

	name = name.substr(0, pos);						//zjisteni IP adresy site
	size = pow(2, 32 - prefix);						//velikost site
	if (prefix <= 30)
	{
		size -= 2;
	}
	count = 0;										//pocet zarizeni v siti

	struct sockaddr_in aux;
	if (prefix < 1 || prefix > 32 || inet_pton(AF_INET, name.c_str(), &(aux.sin_addr)) == 0)		//Prefix je z <1, 32>, adresa je platna
	{
		print_error(ERR_ARGS);
	}

	char ip[4];			//vymaskovani adresy site (pokud je zadana IP zarine v siti misto site)
	to_char(ip, name);
	mask(ip, prefix);

	name = "";

	for (int i = 0; i < 4; i++)								//prevod IP na retezec
	{
		name += to_string((int)(unsigned char)ip[i]) + ".";
	}
	name.pop_back();
	this->name = name;
}

network::network(const network &old)		//copy constructor pro vyuziti vector<>
{
	name = old.name;
	prefix = old.prefix;
	size = old.size;
	count = old.count;
	used = old.used;
	set = old.set;
}

bool network::is_in(string ip)		//urceni jestli IP adresa patri do site
{
	char ip_c[4];
	char nt_c[4];
	to_char(ip_c, ip);
	to_char(nt_c, name);

	if (prefix <= 30 && is_reserved(ip_c, prefix))		//adresa neni adresa site ani broadcast
		return false;

	mask(ip_c, prefix);				//adresa je po vymaskovani shodna s adresou site

	for (int i = 0; i < 4; i++)
	{
		if (ip_c[i] != nt_c[i])
		{
			return false;
		}
	}	
	return true;
}

void network::add(string ip, double now, int lease)
{
	bool found = false;
	if (this->is_in(ip))			//pridavat pouze pokud adresa patri do site
	{
		for (auto i : used)
		{
			if (i.compare(ip) == 0)		//adresa uz v siti je, obnovit lease (RENEW)
			{
				set[ip] = now;
				found = true;
			}
		}
		if (!found)			//pridat adresu
		{
			count++;
			set[ip] = now;
			used.push_back(ip);
		}
	threads.push_back(thread(&network::watcher, &(*this), ip, now, lease));		//vytvorit vlakno hlidajici lease time
	threads.back().detach();
	}
}

void network::remove(string ip)
{
	if (find(used.begin(), used.end(), ip) != used.end())		//odstraneni adresy ze site (RELEASE)
	{
		count--;		//zmensit pocet zarizeni
		set[ip] = 0.0;	//nastavit cas pro watcher (adresa uz je odstranena)
		used.erase(std::remove(used.begin(), used.end(), ip), used.end());			//odstraneni ze seznamu vyuzivanych adres	
	}
}

void network::watcher(string ip, double time, int lease)
{
	this_thread::sleep_for(chrono::seconds(lease));		//uspani na dobu lease
	networks_acces.lock();
	if (set[ip] == time)			//adresa nebyla po dobu spanku uvolnena nebo obnovena (RENEW nebo RELEASE)
	{
		count--;					//odstraneni zaznamu
		set[ip] = 0.0;
		used.erase(std::remove(used.begin(), used.end(), ip), used.end());
	}
	networks_acces.unlock();	
}

void network::print()			//vypis na cout
{
	int pad;

	pad = 21 - (name.size() + 1 + to_string(prefix).size());
	cout << name << "/" << prefix;
	for (int i = 0; i < pad; i++)
	{
		cout << " ";
	}

	pad = 17 - (to_string(size).size());
	cout << size;
	for (int i = 0; i < pad; i++)
	{
		cout << " ";
	}

	pad = 30 - (to_string(count).size());
	cout << count;
	for (int i = 0; i < pad; i++)
	{
		cout << " ";
	}

	cout << fixed;
	cout << setprecision(2);
	cout << 100.0*(double)count/(double)size << '%' << "   " << endl;
}

void network::print_to_file(ofstream &file)		//vypis do log.csv (prepinac -c)
{
	file << name << "/" << prefix << ",";
	file << size << ",";
	file << count << ",";
	file << fixed;
	file << setprecision(2);
	file << (double)count/(double)size << '%' << endl;
}

void network::to_char(char *buf, string ip)		//prevod retezce ip na 4 byte hodnotu 
{
	size_t pos = 0;
	size_t aux;
	for (int i = 0; i < 4; i++)
	{
		buf[i] = stoi(ip.c_str() + pos, &aux);
		pos += aux + 1;
	}
}

void network::mask(char *buf, int mask)		//vymnaskovani adresy site z adresy zarizeni
{
	for (int i = 0; i < 4; i++, mask -= 8)
	{
		int shift = mask;		//prevod hodnoty masky na bitovou masku
		if (shift > 8)
			shift = 8;
		if (shift < 0)
			shift = 0;

		char bit_mask = 0;
		bit_mask = ~bit_mask;
		bit_mask = bit_mask << (8 - shift);
		buf[i] = buf[i] & bit_mask;				//vymakovani bytu
	}
}

bool network::is_reserved(char *buf, int mask)		//zjisteni jestli adresa neni adresou site nebo broadcast
{
	int counter_bc = 0;
	int counter_nt = 0;
	for (int i = 0; i < 4; i++, mask -= 8)
	{
		int shift = mask;		//prevod hodnoty masky na bitovou masku
		if (shift > 8)
			shift = 8;
		if (shift < 0)
			shift = 0;

		char bit_mask = 0;
		bit_mask = ~bit_mask;
		bit_mask = bit_mask << (8 - shift);

		if ((~bit_mask) == ((~bit_mask) & buf[i]))	//byt adresy je broadcast
			counter_bc++;

		if (buf[i] == (bit_mask & buf[i]))		//byt adresy je adresa site
			counter_nt++;
	}

	if (counter_bc == 4 || counter_nt == 4)		//adresa je broadcast nebo adresa site
		return true;
	return false;
}
