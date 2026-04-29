#include <stdlib.h> 
#include <stdio.h>
#include <string.h> 
#include <stdint.h>
#include <time.h>
#include "socket.h"

// Retorna um valor de 8 bits entre min e max, incluindo eles mesmos
uint8_t gera_byte_aleat (uint8_t min, uint8_t max) ;

// Constrói e preenche o frame
unsigned char* build_kermit(unsigned char *buffer_dados, uint8_t tamMsg, uint8_t seq,
                 uint8_t type, uint8_t crc) ; 
