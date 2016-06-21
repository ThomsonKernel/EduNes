#pragma once

class Uint8_t{
public:
    Uint8_t(uint8_t v):value(v){};
    operator uint8_t();
    bool operator[](int);
    uint8_t operator&(int); 
    uint8_t operator|(int);
    uint8_t operator<<(int);
    uint8_t operator>>(int);
    void setTrue(int);
    void setFalse(int);
    void flip(int);
private:
    uint8_t value;
};