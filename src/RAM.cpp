#include <stdint.h>
#include "RAM.h"

uint8_t RAM::read(uint16_t addr)const{
    return ram[addr];
}
void RAM::write(uint16_t addr,uint8_t data){
    ram[addr]=data;
}