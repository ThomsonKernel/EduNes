#include <SDL2/SDL.h>
#include <stdint.h>
#include "SDL_Input.h"

Input::Input()
:
    k(SDL_GetKeyboardState(NULL))
{}

uint8_t Input::read(){
    uint8_t r=state&0x01;
    state>>=1;
    return r;
}
void Input::write(uint8_t data){
    bool s=data&0x01;
    if(!s&&strobe){//H->L
        update_state();
    }
    strobe=s;
}
void Input::update_state(){
    SDL_PumpEvents();//1:start 2:select D:B_button F:A_button
    state=k[SDL_SCANCODE_RIGHT]<<7|k[SDL_SCANCODE_LEFT]<<6|k[SDL_SCANCODE_DOWN]<<5|k[SDL_SCANCODE_UP]<<4|k[SDL_SCANCODE_1]<<3|k[SDL_SCANCODE_2]<<2|k[SDL_SCANCODE_D]<<1|k[SDL_SCANCODE_F];
}
