#pragma once
#include <fstream>

class Cart{
public:
    Cart(std::ifstream&);
    ~Cart();
    uint8_t prg_read(uint16_t addr)const;
    void prg_write(uint16_t addr,uint8_t data);

    uint8_t chr_read(uint16_t addr)const;
    void chr_write(uint16_t addr,uint8_t data);
private:
    uint8_t* prg_rom;
    static constexpr int PRG_UNIT=0x4000;//16384

    uint8_t* chr_rom;
    static constexpr int CHR_UNIT=0x2000;//8192

    uint8_t vram[0x0800];
    uint8_t* screen[4];

    enum Mirroring_mode{
        HORIZONTAL,
        VERTICAL
    };
};