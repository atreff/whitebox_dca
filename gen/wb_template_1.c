#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

extern const uint8_t Tbox[256];

int main(int argc, char* argv[]){
    uint8_t result = { 0 };
    char buf[2] = { argv[1][0], argv[1][1]};
    uint8_t input = (uint8_t)strtol(buf, NULL, 16);
    result = Tbox[input];
    printf("%02x\n", result);
}

