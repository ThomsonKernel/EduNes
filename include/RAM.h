#pragma once

class RAM {
public:
    uint8_t read(uint16_t addr)const;
    void write(uint16_t addr,uint8_t value);
private:
    static constexpr int size = 0x0800;
    uint8_t ram[size];
};