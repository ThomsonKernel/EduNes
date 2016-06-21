#include "SDL_Video.h"

constexpr uint8_t Renderer::rgb_pallet[0x200][DEPTH];
void Renderer::render(int V,int H,int pallet_data_with_tint){//9bit
    memcpy(pixels[V][H],rgb_pallet[ pallet_data_with_tint ],DEPTH);
}

void Renderer::update(){
    void *p;
    int pitch;
    SDL_LockTexture(tex,&srcrect,&p,&pitch);
    SDL_memcpy(p,pixels,HEIGHT*WIDTH*DEPTH);
    SDL_UnlockTexture(tex);
    SDL_RenderCopy(ren,tex,&srcrect,&dstrect);
    SDL_RenderPresent(ren);
}