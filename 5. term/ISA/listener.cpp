/*
 * Autor: Ondrej Vales, xvales03
 * Predmet: ISA 2016/2017
 * Projekt: Monitorovani DHCP komunikace
 */

#include "listener.h"
#include "error.h"

listener::listener()
{
	if ((identif = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) <= 0)		// vytvor socket
	{
		print_error(ERR_SOCK);
	}
}

void listener::listen(message &mes)
{
	char buf[BUFFER_SIZE];
	ssize_t size;
	int offset;
	struct iphdr *ip_header;
	do
	{
		do
		{
			size = recv(identif, buf, BUFFER_SIZE, 0);
			if (size < 0)
			{
				print_error(ERR_RECV);
			}
			ip_header = (struct iphdr*)(buf + sizeof(struct ethhdr));
		} while (ip_header->protocol != UDP);		//preskakovani ne-UDP zprav
		offset = ntohs(((struct udphdr*)(buf + ip_header->ihl * 4  + sizeof(struct ethhdr)))->dest);
	} while (offset != DHCP_ACK && offset != DHCP_REL);		//preskakovani ne-DHCP zprav

	offset = sizeof(struct ethhdr) + ip_header->ihl * 4 + sizeof(struct udphdr);		//odstraneni Eth, IP a UDP hlavicky

	size -= offset;
	mes.copy(buf + offset, size);
}

listener::~listener()		//zavreni socketu
{
	close(identif);
}
