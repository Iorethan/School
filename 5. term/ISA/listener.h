/*
 * Autor: Ondrej Vales, xvales03
 * Predmet: ISA 2016/2017
 * Projekt: Monitorovani DHCP komunikace
 */

#ifndef SOCKET_CLASS_FILE
#define SOCKET_CLASS_FILE

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <cstring> 

#include "buffer.h"
#include "message.h"

#define UDP 17
#define DHCP_ACK 68
#define DHCP_REL 67

using namespace std;

class listener
{
	int identif;

public:
	listener();
	void listen(message &mes);
	~listener();
};

#endif
