#include <stdint.h>
#include "Uint8_t.h"

Uint8_t::operator uint8_t(){
    return value;
}
uint8_t Uint8_t::operator &(int i){
    return value&i;
}
uint8_t Uint8_t::operator |(int i){
    return static_cast<uint8_t>(value|i);
}
uint8_t Uint8_t::operator <<(int i){
    return static_cast<uint8_t>(value<<i);
}
uint8_t Uint8_t::operator >>(int i){
    return value>>i;
}
bool Uint8_t::operator[](int i){
    return value& 1<<i;
}
void Uint8_t::setTrue(int i){
    value |= 1<<i;
}
void Uint8_t::setFalse(int i){
    value &= ~(1<<i);
}
void Uint8_t::flip(int i){
    value ^= 1<<i;
}