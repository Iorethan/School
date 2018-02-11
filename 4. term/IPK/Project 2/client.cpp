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

#define SZ 1024

using namespace std;

bool check_args(int argc, char** argv, int &port, char* &host_name, char* &file_name, bool &d);
bool set_up(int &clsoc, int port, struct sockaddr_in &server_addr, struct hostent* &server, char* host_name);
bool comunicate(int comsoc, bool d, char* file_name);
char* get_file_name(char* file_name);
bool download_request(int comsoc, char* file);
bool upload_request(int comsoc, char* file, char* file_name);
void send_noop(int comsoc);

int main (int argc, char** argv)
{
	int clsoc, port;
	char* host_name, * file_name;
	struct hostent* server;
	struct sockaddr_in server_addr;
	bool d = false;
		
	if (!check_args(argc, argv, port, host_name, file_name, d))		//kontrola argumentu
	{
		std::cerr << "Chyba: ARG\n";
		return -1;
	}
	
	if (!set_up(clsoc, port, server_addr, server, host_name))		//navazani spojeni se serverem
	{
		return -1;
	}
	
	if (!comunicate(clsoc, d, file_name))							//stahnuti/nahrani souboru
	{
		return -1;
	}
	return 0;
}

bool check_args(int argc, char** argv, int &port, char* &host_name, char* &file_name, bool &d)
{
	if (argc != 7)
	{
		return false;
	}
	
	bool p = false, h = false, du = false;		//kontrola unikatnosti zadanyh argumentu
	
	for (int i = 1; i < 7; i++)
	{
		if (strcmp(argv[i], "-p") == 0 && !p)	//argument -p port
		{
			p = true;
			i++;
			char* endptr;
			port = strtol(argv[i], &endptr, 10);			
			if (*endptr != '\0')
			{
				return false;
			}	
			if (argv[i][0] < '1' || argv[i][0] > '9')
			{
				return false;
			}	
		}
		else if (strcmp(argv[i], "-h") == 0 && !h)	//argument -h hostname
		{
			h = true;
			i++;
			host_name = argv[i];	
		}
		else if (((d = (strcmp(argv[i], "-d") == 0)) || strcmp(argv[i], "-u") == 0) && !du) //argument -d/-u filename
		{
			du = true;
			i++;
			file_name = argv[i];
		}
		else
		{
			return false;
		}
	}
	
	return true;
}

bool set_up(int &clsoc, int port, struct sockaddr_in &server_addr, struct hostent* &server, char* host_name)
{
	if ((server = gethostbyname(host_name)) == NULL)		//prelozeni jmena serveru
	{
		std::cerr << "Chyba: DNS\n";
		return false;
	}
	
	if ((clsoc = socket(AF_INET, SOCK_STREAM, 0)) <= 0)		//vytvoreni socketu
	{
		std::cerr << "Chyba: SOC\n";
		return false;
	}
	
	memset((void*)&server_addr, 0, sizeof(server_addr));		//naplneni struktury s udaji p serveru
	server_addr.sin_family = AF_INET;
	bcopy((char*)server->h_addr, (char*)&server_addr.sin_addr.s_addr, server->h_length);	
	server_addr.sin_port = htons(port);
	
	if (connect(clsoc, (const struct sockaddr*)&server_addr, sizeof(server_addr)) != 0)		//navazani spojeni
    {
		close(clsoc);
		std::cerr << "Chyba: CON\n";
		return false;
	}
	return true;
}

bool comunicate(int comsoc, bool d, char* file_name)
{
	char* file = get_file_name(file_name);	//zjisteni jmena souboru
	if (file == NULL)	//pokud je jmeno neplatne ukonci spojeni
	{
		send_noop(comsoc);
		close(comsoc);
		std::cerr << "Chyba: ARG\n";
		return false;		
	}
	
	if (d)	//dowload/upload
	{
		if (file != file_name)	//soubor se nenachazi v adresari serveru, ukonci spojeni
		{
			send_noop(comsoc);
			close(comsoc);
			std::cerr << "Chyba: ARG\n";
			return false;
		}
		if (!download_request(comsoc, file))	//odesli pozadavek na stazeni
		{
			return false;			
		}
	}
	else
	{
		if (!upload_request(comsoc, file, file_name))	//odesli pozadavek pro upload
		{
			return false;		
		}
	}
	
	close(comsoc);	//zavri socket
	return true;
}

char* get_file_name(char* file_name)
{
	string fn;
	fn.assign(file_name);
	int pos = fn.size();
	if (file_name[pos - 1] == '/')		//nalezeni posledniho / v ceste k souboru (za ni nasleduje jmeno souboru)
	{
		return NULL;
	}
	pos = fn.find_last_of("/");
	if (pos == string::npos)
	{
		return file_name;
	}
	return file_name + pos + 1;	
}

bool download_request(int comsoc, char* file)
{
	char buff[SZ];
	string fl;
	fl.assign(file);
	string message = "OVP 100 DOWNLOAD\n" + fl + "\n\n";	//odeslani zpravy pro download
	
	int res = send(comsoc, message.c_str(), message.size(), 0);		//chyba pri odeslani zpravy
	if (res < 0)
	{
		close(comsoc);
		std::cerr << "Chyba: SNT\n";
		return false;		
	}
	
	char c;
	bool last = false;
	message = "";
	res = recv(comsoc, &c, 1, 0);		//cekani na odpoved
	while (res > 0)					//precteni odpovedi (jen hlavicka)
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
	if (res < 0)	//chyba pri cteni odpovedi
	{
		close(comsoc);
		std::cerr << "Chyba: REC\n";
		return false;		
	}
	
	if (message.size() > 7)		//server nemohl poslat soubor
	{
		if (message.substr(4, 3) == "400")
		{
			close(comsoc);
			std::cerr << "Chyba: OPN\n";
			std::cerr << "soubor neexistuje\n";
			return false;	
		}
	}
	
	message = message.substr(message.find("\n") + 1);	//zjisteni jmena a delky
	string fn = message.substr(0, message.find("\n"));
	message = message.substr(message.find("\n") + 1);
	
	int lenght;
	sscanf(message.c_str(), "%d", &lenght);
		
	ofstream offile (fn.c_str(), ios::out | ios::binary);	//otevreni souboru pro zapis
	if (!offile.is_open())
	{
		close(comsoc);
		std::cerr << "Chyba: OPN\n";
		std::cerr << "nelze zapsat soubor\n";
		return false;	
	}
	
	res = recv(comsoc, buff, SZ, 0);		//nacteni a zapis souboru
	while (res > 0 && lenght > 0)
	{
		if (res > lenght && lenght > 0)
		{
			if (res != lenght + 1 || buff[lenght] != '\n')
			{
				close(comsoc);		//spatna delka souboru
				offile.close();
				std::cerr << "Chyba: DAT\n";
				std::cerr << "nelze zapsat soubor\n";
				return false;
			}
			else
			{
				offile.write(buff, lenght);	//zapis posledni casti, kontrola delky
				offile.close();
				close(comsoc);
				lenght -= res;
			}
		}
		else
		{
			offile.write(buff, res);		//zapis
			lenght -= res;
			res = recv(comsoc, buff, SZ, 0);			
		}
	}
	
	return true;
}

bool upload_request(int comsoc, char* file, char* file_name)
{
	char buff[SZ] = {0};
	string fl;
	fl.assign(file);
	string content = "";
	ifstream infile (file_name, ios::in | ios::binary);	//otevreni souboru pro cteni
	if(!infile.is_open())
	{
		send_noop(comsoc);		//chyba pri otevreni souboru
		close(comsoc);
		std::cerr << "Chyba: OPN\n";
		std::cerr << "soubor neexistuje nebo nejde otevrit\n";
		return false;		
	}
	int c = infile.get();		//nacteni souboru pro odeslani
	while(!infile.eof())
	{
		content.append(1, c);
		c = infile.get();
	}
	infile.close();
	int l = content.size();		//zjisteni delky
	sprintf(buff, "%d", l);
	string len;
	len.assign(buff);
	
	string message = "OVP 200 UPLOAD\n" + fl + "\n" + len + "\n\n" + content + "\n";		//zprava s prilozenou delkou a souborem
	
	int res = send(comsoc, message.c_str(), message.size(), 0);		//odeslani zpravy
	if (res < 0)
	{
		close(comsoc);
		std::cerr << "Chyba: SNT\n";
		return false;		
	}
	
	message = "";
	res = recv(comsoc, buff, SZ, 0); //cekani na odpoved serveru
	while (res > 0)
	{
		message.append(buff, res);
		res = recv(comsoc, buff, SZ, 0);
	}
	if (res < 0)
	{
		close(comsoc);
		std::cerr << "Chyba: REC\n";
		return false;		
	}
	
	if (message.size() > 7)
	{
		if (message.substr(4, 3) == "500")		//chyba pri zapisu na strane serveru
		{
			close(comsoc);
			std::cerr << "Chyba: OPN\n";
			std::cerr << "soubor nebylo mozne zapsat\n";
			return false;	
		}
	}
	
	return true;
}

void send_noop(int comsoc)	//odeslani zpravy pro ukonceni spojeni
{
		string message = "OVP 000 NO_OPERATION\n\n";
		int res = send(comsoc, message.c_str(), message.size(), 0);
		return;
}