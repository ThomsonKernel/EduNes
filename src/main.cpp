#include <iostream>
#include <fstream>
#include <SDL2/SDL.h>
#include "SDL_Video.h"
#include "CPU.h"
#include "PPU.h"
#include "Cart.h"

using namespace std;

namespace {
    SDL_Window* win;
    SDL_Renderer* ren;
    SDL_Texture* tex;
};

bool sdl_init(){
    if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO) != 0){
        cerr << "SDL Error: " << SDL_GetError() << '\n';
        return false;
    }
    //
    win=SDL_CreateWindow("EduNes", SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,WIDTH*MAG,HEIGHT*MAG,SDL_WINDOW_SHOWN);
    if(!win){
         cerr << "SDL Error: " << SDL_GetError() << '\n';
        SDL_Quit();
        return false;
    }
    ren=SDL_CreateRenderer(win,-1, SDL_RENDERER_SOFTWARE);
    if(!ren){
        SDL_DestroyWindow(win);
        cerr << "SDL Error: " << SDL_GetError() << '\n';
        SDL_Quit();
        return false;
    }
    tex=SDL_CreateTexture(ren, SDL_PIXELFORMAT_RGB24,SDL_TEXTUREACCESS_STREAMING,WIDTH,HEIGHT);
    if(!tex){
        SDL_DestroyRenderer(ren);
        SDL_DestroyWindow(win);
        cerr << "SDL Error: " << SDL_GetError() << '\n';
        SDL_Quit();
        return false;
    }
    return true;
}

void sdl_quit(){
    SDL_DestroyTexture(tex);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
}

int main(int argc, char const *argv[]){
    ifstream ifs(argv[1],ios::in|ios::binary);
    if(!ifs){
        cerr << "No such file" << '\n';
        return -1;
    }


    Cart *cart;
    try{
        cart = new Cart(ifs);
    }catch(const char* const str){
        cerr << str << '\n';
        return -1;
    }

    if(!sdl_init())
        return -1;
    Renderer r(ren,tex);

    PPU ppu(cart,r);
    CPU cpu(cart,ppu);

    unsigned short count=0;
    while(1){
        ppu.run();
        ppu.run();
        ppu.run();
        cpu.run();
        if(!++count){
            SDL_Event ev;
            while(SDL_PollEvent(&ev)){
                if(ev.type==SDL_QUIT)
                    goto Quit;
            }
        }
    }
Quit:
    sdl_quit();
    delete cart;
    return 0;
}