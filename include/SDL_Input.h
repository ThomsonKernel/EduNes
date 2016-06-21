#pragma once

class Input
{
public:
    bool strobe;
    uint8_t state;
    const uint8_t* k;//keybord
    Input();
    uint8_t read();
    void write(uint8_t data);
private:
    void update_state();
};