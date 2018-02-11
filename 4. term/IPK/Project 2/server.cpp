#include <string.h>
#include <string>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <thread>

#define SZ 1024

using namespace std;

bool check_args(int argc, char** argv, int &port);
bool set_up(int &ssoc, int port, struct sockaddr_in &server_addr);
void comunicate(int comsoc);
void download_request(int comsoc, string &message);
void upload_request(int comsoc, string &message);

bool silent = true; 	//promenna potlacujici ladici vypisy

int main (int argc, char** argv)
{
	int ssoc, port;
	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;
	socklen_t len = sizeof(client_addr);

	if (!check_args(argc, argv, port))		//kontrola argumentu
	{
		std::cerr << "Chyba: ARG\n";
		return -1;
	}
	
	if (!set_up(ssoc, port, server_addr))	//vytvoreni socketu, navazani na socket a zacatek naslouchani na portu
	{
		return -1;
		close(ssoc);
	}
	
	while (true)	//nekonecna smycka obsluhujici prichozi pozadavky
	{
		int comsoc = accept(ssoc, (struct sockaddr*)&client_addr, &len);		//ziskani cisla socketu na kterem bude probihat komunikace
		if (comsoc <= 0)
		{
			std::cerr << "Chyba: ACC\n";			
			close(ssoc);
			return -1;
		}
		thread (comunicate, comsoc).detach();		//vytvoreni vlakna pro kromunikaci s jednim klientem
	}
	
	return 0;
}

bool check_args(int argc, char** argv, int &port)
{
	if (argc != 3 || strcmp(argv[1], "-p") != 0)	//ocekavam dva argumenty zadane uzivatelem
	{
		return false;
	}
	
	if (argv[2][0] < '1' || argv[2][0] > '9')		//prevod zadaneho portu na cislo
	{
		return false;
	}
	
	char* endptr;
	port = strtol(argv[2], &endptr, 10);
	
	if (*endptr != '\0')
	{
		return false;
	}
	
	return true;
}

bool set_up(int &ssoc, int port, struct sockaddr_in &server_addr)
{
	if ((ssoc = socket(AF_INET, SOCK_STREAM, 0)) <= 0)		//vytvoreni socketu
	{
		std::cerr << "Chyba: SOC\n";
		return false;
	}
	
	memset((void*)&server_addr, 0, sizeof(server_addr));	//nastaveni struktury s adresou serveru
	server_addr.sin_family = AF_INET;	
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);	
	server_addr.sin_port = htons(port);
	
	if (bind(ssoc, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)	//navazani na socket
	{
		std::cerr << "Chyba: BND\n";
		return false;
	}
	
	if (listen(ssoc, 1) < 0)		//naslouchani na socketu
	{
		std::cerr << "Chyba: LIS\n";
		return false;			
	}
	return true;
}

void comunicate(int comsoc)
{
	if (!silent)
	{
		cout << "T " << comsoc << " starts\n";
		sleep(5);
	}
	int res = 0;
	char c;
	string message = "";		//string do ktereho je ukladana prijata zprava
	bool last = false;			//hlavicka ukoncena "\n\n"
	res = recv(comsoc, &c, 1, 0);
	while (res > 0)		//nacteni cele hlavicky (pouze hlavicky)
	{	
		message.append(1, c);
		if (c == '\n')
		{
			if (last)
			{
				res = 0;
			}
			else
			{
				last = true;
				res = recv(comsoc, &c, 1, 0);
			}
		}
		else
		{
			last = false;
			res = recv(comsoc, &c, 1, 0);
		}
	}
	
	if (res < 0)	//pri nacitani doslo k chybe
	{
		close(comsoc);
		std::cerr << "Chyba: REC\n";
		return;		
	}
	
	if (message.size() > 7)		//urceni typu prijate zpravy
	{
		if (message.substr(4, 3) == "100")
		{
			download_request(comsoc, message);	//pozadavek na stazeni souboru
		}
		else if (message.substr(4, 3) == "200")
		{
			upload_request(comsoc, message);	//pozadavek na nahrani souboru
		}
	}
	
	close(comsoc);	//uzavreni socketu
	if (!silent)
	{
		cout << "T " << comsoc << " ends\n";		
	}
	return;
}

void download_request(int comsoc, string &message)
{
	char buff[SZ] = {0};
	int res;
	string content = "";		//string do ktereho je ulozena prenasena zprava
	message = message.substr(message.find("\n") + 1);		//precteni jmena souboru z pozadavku
	string fn = message.substr(0, message.find("\n"));
	ifstream infile (fn.c_str(), ios::in | ios::binary);	//otevreni souboru
	
	if (!infile.is_open())	//nepodarilo se otevrit pozadovany soubor
	{
		message = "OVP 400 UNABLE_TO_FIND\n\n";
		res = send(comsoc, message.c_str(), message.size(), 0);	//odeslani chybove zpravy klientovi
		if (res < 0)
		{
			close(comsoc);
			std::cerr << "Chyba: SNT\n";
			return;
		}
		close(comsoc);
		return;
	}
	
	int c = infile.get();		//nacteni prenaseneho souboru
	while(!infile.eof())
	{
		content.append(1, c);
		c = infile.get();
	}
	infile.close();
	int l = content.size();		//zjisteni delky zpravy
	sprintf(buff, "%d", l);
	string len;
	len.assign(buff);
	
	message = "OVP 300 OK\n" + fn + "\n" + len + "\n\n" + content + "\n";		//zprava k odeslani
	
	res = send(comsoc, message.c_str(), message.size(), 0);		//odeslani zpravy s pripojenym souborem
	if (res < 0)
	{
		close(comsoc);
		std::cerr << "Chyba: SNT\n";
		return;		
	}
	return;
}

void upload_request(int comsoc, string &message)
{
	char buff[SZ];
	int res;
	
	message = message.substr(message.find("\n") + 1);		//precteni jmena souboru z pozadavku
	string fn = message.substr(0, message.find("\n"));
	message = message.substr(message.find("\n") + 1);		//precteni delky prijateho souboru
	
	int lenght;
	sscanf(message.c_str(), "%d", &lenght);		//prevedeni delky
	
	ofstream offile (fn.c_str(), ios::out | ios::binary);	//otevreni souboru pro zapis
	if (!offile.is_open())		//nelze otevrit soubor
	{
		message = "OVP 500 UNABLE_TO_WRITE\n\n";
		res = send(comsoc, message.c_str(), message.size(), 0);
		if (res < 0)
		{
			close(comsoc);
			std::cerr << "Chyba: SNT\n";
			return;
		}		
	}
	
	res = recv(comsoc, buff, SZ, 0);		//precteni prijateho souboru
	while (res > 0)
	{
		if (res > lenght && lenght > 0)
		{
			if (res != lenght + 1 || buff[lenght] != '\n')	//kontrola zda byl soubor precten cely
			{
				message = "OVP 500 UNABLE_TO_WRITE\n\n";
				res = send(comsoc, message.c_str(), message.size(), 0);
				if (res < 0)
				{
					close(comsoc);
					std::cerr << "Chyba: SNT1\n";
					return;
				}
				
				offile.close();
				close(comsoc);
				std::cerr << "Chyba: DAT\n";
				return;
			}
			else
			{
				offile.write(buff, lenght);		//zapis posledni casti souboru
				offile.close();
				lenght -= res;
				message = "OVP 300 OK\n\n";		//odeslani potvrzeni zapisu
				res = send(comsoc, message.c_str(), message.size(), 0);
				if (res < 0)
				{
					close(comsoc);
					std::cerr << "Chyba: SNT\n";
					return;
				}
				res = 0;
				close(comsoc);
			}
		}
		else
		{
			offile.write(buff, res);	//zapis casti souboru
			lenght -= res;
			res = recv(comsoc, buff, SZ, 0);			
		}
	}
	
	return;
}