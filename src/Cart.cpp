#include <stdint.h>//uint8_t
#include "Cart.h"

Cart::Cart(std::ifstream& ifs)
{
    uint8_t header[16];
    ifs.read(reinterpret_cast<char *>(header),16);
    uint8_t mapper = header[6]>>4 | (header[7] & 0xF0);
    if(mapper)
        throw "Unsupported mapper.";
    if(header[6]&0x04)
        throw "Trainer is not supported.";
    if(header[6]&0x02)
        throw "Save RAM is not supported.";
    if(header[7]&0x02)
        throw "PlayChoise is not supported.";
    if(header[7]&0x01)
        throw "Vs Unisystem is not supported.";

    //mapper 0
    prg_rom=new uint8_t[PRG_UNIT*2];
    ifs.read(reinterpret_cast<char *>(prg_rom),header[4]*PRG_UNIT);
    if(header[4]==1)
        memcpy(&prg_rom[PRG_UNIT],prg_rom,PRG_UNIT);

    chr_rom=new uint8_t[CHR_UNIT];
    ifs.read(reinterpret_cast<char *>(chr_rom),CHR_UNIT);

    switch(header[6]&0x01){
        case HORIZONTAL:
                screen[0]=vram;
                screen[1]=vram;
                screen[2]=&vram[0x400];
                screen[3]=&vram[0x400];
            break;
        case VERTICAL:
                screen[0]=vram;
                screen[1]=&vram[0x400];
                screen[2]=vram;
                screen[3]=&vram[0x400];
    }

    ifs.close();
}
Cart::~Cart(){
    delete[] prg_rom;
    delete[] chr_rom;
}

uint8_t Cart::prg_read(uint16_t addr)const{
    if(addr&0x8000){//$8000-$FFFF
        return prg_rom[addr&0x7FFF];
    }else{//$4200-$7FFF
        throw "error";
    }
}
void Cart::prg_write(uint16_t addr,uint8_t data){

}

uint8_t Cart::chr_read(uint16_t addr)const{
    if(addr&0x2000){//$2000-3FFF
        return screen[addr>>10&0x03][addr&0x03FF];
    }else{//$0000-1FFF
        return chr_rom[addr];
    }
}
void Cart::chr_write(uint16_t addr,uint8_t data){
    if(addr&0x2000){//$2000-3FFF
        screen[addr>>10&0x03][addr&0x03FF]=data;
    }
}