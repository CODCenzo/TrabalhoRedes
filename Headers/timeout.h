#ifndef _TIMEOUT_H_
#define _TIMEOUT_H_

#include <arpa/inet.h>
#include <net/ethernet.h>
#include <linux/if_packet.h>
#include <sys/time.h>
#include <net/if.h>
#include <stdlib.h>
#include <stdio.h>

long long timestamp();

int protocolo_e_valido(unsigned char* buffer, int tamanho_buffer);

int recebe_mensagem(int soquete, int timeoutMillis, unsigned char* buffer, int tamanho_buffer);

#endif
