#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
//#include "enctbox.h"

extern const uint8_t Tbox[256];

int main(int argc, char* argv[]){
    uint8_t result[16] = { 0 };
    for(int i = 0; i < 16; i++) {
        char buf[2] = { argv[1][i*2], argv[1][i*2+1]};
        uint8_t input = (uint8_t)strtol(buf, NULL, 16);
        result[i] = Tbox[input];
        printf("%02x", result[i]);
    }
    printf("\n");
}

