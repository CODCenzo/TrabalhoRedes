#ifndef _SOCKET_H_
#define _SOCKET_H_

#include <arpa/inet.h>
#include <net/ethernet.h>
#include <linux/if_packet.h>
#include <net/if.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

int cria_raw_socket(char* nome_interface_rede);

#endif
