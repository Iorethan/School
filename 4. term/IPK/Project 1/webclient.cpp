#include <string.h>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>

#define SZ 8000
#define PORT 80

int parse_url(std::string &arg, std::string &name, std::string &dir, std::string &save, int &port);
int translate_connect(struct hostent* &server, struct sockaddr_in &server_addr, const std::string &name, const std::string &dir, const int &port, int &clsoc);
int send_req(std::string &arg, const std::string &name, const std::string &dir, const int &clsoc);
int send_req_0(std::string &arg, const std::string &name, const std::string &dir, const int &clsoc);
int get_head(std::string &arg, const int &clsoc, bool &succ, std::string &save, bool &resend);
int get_code(const std::string &arg);
void get_url(std::string &arg);
int parse_data(std::string &data, std::string &save, size_t pos);
void dechunk(std::string &data);

bool first = true;

int main (int argc, char** argv)
{
	int clsoc, port = PORT;
	struct hostent* server;
	struct sockaddr_in server_addr;
	std::string arg, name, dir, save;
	bool succ = false;
	
	if(argc != 2)
	{
		std::cerr << "Chyba: ARG\n";
		return 1;
	}
	
	for (int i = 0; i < 6 && !succ; i++)	// cyklus pro navazovani spojeni, prvni + 5x redirect
	{
		//std::cerr << "Cycle " << i + 1 << std::endl << "==========\n";
		if (i == 0)		// poprve adresa v argumentu
			arg.assign(argv[1]);
		
		if (parse_url(arg, name, dir, save, port) != 0)		// rozloz adresu na jmeno serveru, soubor a port
			return 1;
		
		if (translate_connect(server, server_addr, name, dir, port, clsoc) != 0)	// navaz spojeni
			return 1;
		
		bool resend = false;
		for (int req = 0; req < 2 && (resend || req == 0); req++)		// odesli http 1.1 pozadavek, pokud se nepovede poslo 1.0
		{
			if (req == 0)
			{
				if (send_req(arg, name, dir, clsoc) != 0)
					return 1;
			}
			else
			{
				if (send_req_0(arg, name, dir, clsoc) != 0)
					return 1;
			}
			
			if (get_head(arg, clsoc, succ, save, resend) != 0)		// precti obsah hlavicky
				return 1;
		}			
		
		close(clsoc);
		
		//std::cerr << "==========\n\n";
	}
	
	if (!succ)
	{
		std::cerr << "Chyba: RDR\n";
		return 1;
	}
	return 0;
}

int parse_url(std::string &arg, std::string &name, std::string &dir, std::string &save, int &port)
{	
	size_t pos;
	
	if (arg.find("http://") == 0)		// odstran http://
		arg.erase(0, 7);
	
	pos = arg.find(".");				// najdi predel mezi jmenem serevru a cestou k souboru (prvni lomitko za prvni teckou)
	if (pos == std::string::npos)
	{
		std::cerr << "Chyba: PTH\n";
		return 1;
	}	
	pos = arg.find("/", pos);
	
	name = arg.substr(0, pos);
	if (pos == std::string::npos)		// prazdna cesta k souboru
		dir = "/";	
	else
		dir = arg.substr(pos);
	
	pos = name.find(":");				// osetreni explicitne zadaneho portu
	if (pos != std::string::npos)
	{
		arg = name.substr(pos);
		name = name.substr(0, pos);
		try
		{
			arg = arg.substr(1);
			if (arg.size() == 0)
				throw 1;
			sscanf(arg.c_str(), "%d", &port);
		}
		catch (int err)
		{
			std::cerr << "Chyba: PTH\n";
			return 1;
		}
	}
	
	if (first)
	{
		first = false;
		if (dir[dir.size() - 1] == '/')		// nebylo zadano jmeno souboru
			save = "index.html";
		else 
			save = dir.substr(dir.find_last_of("/") + 1);

		pos = (save.find("?"));				// ignoruj predavane parametry
		if (pos == 0)
			save = "index.html";
		else
			save = save.substr(0, pos);
	}
	
	while ((pos = dir.find(" ")) != std::string::npos)		// nahrad mezery (ostatni symboly nejsou nahrazovany)
	{
		dir.erase(pos, 1);
		dir.insert(pos, "%20");
	}
	
	
	while ((pos = dir.find("~")) != std::string::npos)		// nahrad vlnovky (ostatni symboly nejsou nahrazovany)
	{
		dir.erase(pos, 1);
		dir.insert(pos, "%7E");
	}
	
	//std::cerr << name << std::endl << dir << std::endl << port << std::endl << save << std::endl;
		return 0;
}

int translate_connect(struct hostent* &server, struct sockaddr_in &server_addr, const std::string &name, const std::string &dir, const int &port, int &clsoc)
{		
	if ((clsoc = socket(AF_INET, SOCK_STREAM, 0)) <= 0)		// vytvor socket
	{
		std::cerr << "Chyba: SOC\n";
		return 1;
	}
	
	server = gethostbyname(name.c_str());		// preloz jmeno serveru
	if (server == NULL)
	{
		std::cerr << "Chyba: DNS\n";
		return 1;
	}
	
	bzero((char*)&server_addr, sizeof(server_addr));
	
	server_addr.sin_family = AF_INET;
	
	bcopy((char*)server->h_addr, (char*)&server_addr.sin_addr.s_addr, server->h_length);
	
	server_addr.sin_port = htons(port);
	
	if (connect(clsoc, (const struct sockaddr*)&server_addr, sizeof(server_addr)) != 0)		// navaz spojeni se serverem
    	{
		std::cerr << "Chyba: CON\n";
		return 1;
	}
	return 0;
}

int send_req(std::string &arg, const std::string &name, const std::string &dir, const int &clsoc)	// posli http 1.1 dotaz
{	
	arg = "GET " + dir + " HTTP/1.1\r\nHost: " + name + "\r\nConnection: close\r\n\r\n";
	
	if (send(clsoc, arg.c_str(), arg.size(), 0) < 0)
	{
		std::cerr << "Chyba: SNT\n";
		return 1;
	}	
	return 0;
}

int send_req_0(std::string &arg, const std::string &name, const std::string &dir, const int &clsoc)	// posli http 1.0 dotaz
{	
	arg = "GET " + dir + " HTTP/1.0\r\nHost: " + name + "\r\n\r\n";
	
	if (send(clsoc, arg.c_str(), arg.size(), 0) < 0)
	{
		std::cerr << "Chyba: SNT\n";
		return 1;
	}	
	return 0;
}

int get_head(std::string &arg, const int &clsoc, bool &succ, std::string &save, bool &resend)	// nacti projatou hlavicku
{
	char buff[SZ];
	size_t pos;
	int code;
	
	arg = "";
	
	do 
	{
		if ((code = recv(clsoc, buff, SZ, 0)) < 0)
		{
			std::cerr << "Chyba: RCV\n";
			return 1;
		}
		arg.append(buff, code);
	}
	while ((pos = arg.find("\r\n\r\n")) == std::string::npos);		// nacitej data dokud nenarazis na prazdny radek (konec hlavicky)
	
	code = get_code(arg.substr(9, 3));			// precti navratovy kod
	if (code == 301 || code == 302)				// presmerovani
	{
		get_url(arg);	
		//std::cerr << code << " RD" << std::endl << arg << std::endl;
	}
	else if (code >= 400 && code < 500)						// posli http 1.0 dotaz
	{
		resend = true;
		//std::cerr << code << " V0" << std::endl << arg << std::endl;
	}
	else if (code == 200)						// vse vporadku
	{
		succ = true;
		//std::cerr << "200 OK" << std::endl;
		while ((code = recv(clsoc, buff, SZ, 0)) != 0)		// precti cely obsah
		{
			if (code < 0)
			{
				std::cerr << "Chyba: RCV\n";
				return 1;
			}
			arg.append(buff, code);
		}
		if (parse_data(arg, save, pos) != 0)			// zpracuj data
			return 1;
	}
	else
	{
		std::cerr << "Chyba: " << code << std::endl;
		return 1;
	}

	return 0;
}

int get_code(const std::string &arg)		// transformace retezce kodu na int
{
	int code;
	sscanf(arg.c_str(), "%d", &code);
	return code;
}

void get_url(std::string &arg)				// nova adresa po presmerovani
{
	size_t pos;
	pos = arg.find("Location: ") + 10;
	arg = arg.substr(pos, arg.find("\r\n", pos) - pos);
	return;
}

int parse_data(std::string &data, std::string &save, size_t pos)
{
	if (data.compare(pos, 4, "\r\n\r\n"))
	{
		std::cerr << "Chyba: DAT\n";
		return 1;
	}
	
	bool chunked;
	std::string head;
	
	head = data.substr(0, pos + 3);		// hlavicka
	chunked = head.find("Transfer-Encoding: chunked") != std::string::npos;
	data = data.substr(pos + 4);		// odstran hlavicku
	/*std::cerr << chunked << std::endl;
	std::cerr << data.size() << std::endl;*/
	if (chunked)		// prisla chunked odpoved
		dechunk(data);
	
	std::fstream file;
	file.open(save.c_str(), std::ios_base::out | std::ios_base::binary);
	file.write(data.c_str(), data.size());
	file.close();
	return 0;
}

void dechunk(std::string &data)		// sloz chunked odpoved dohromady
{
	int chunks = 0;
	unsigned int size;
	size_t pos = 0;
	
	do
	{
		//std::cerr << "\t" << data.substr(pos, data.find("\r\n") - pos) << std::endl;		// najdi velikost
		size = strtoul((data.substr(pos, data.find("\r\n") - pos)).c_str(), NULL, 16);		// preved na decimalni cislo
		//std::cerr << "\t" << pos << "\t" << size << std::endl;
		data.erase(pos, data.find("\r\n") + 2 - pos);				// odstran metadata
		pos += size;												// posun se za chunk
		data.erase(pos, 2);
	} while (size != 0);			// posledni chunku ma velikost 0
	return;
}
