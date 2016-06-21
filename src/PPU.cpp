#include <stdint.h>
#include <string.h>
#include "Cart.h"
#include "PPU.h"

#define _HORIZONTAL_INC_ case 8:case 16:case 24:case 32:case 40:case 48:case 56:case 64:case 72:case 80:case 88:case 96:case 104:case 112:case 120:case 128:case 136:case 144:case 152: case 160:case 168:case 176:case 184:case 192:case 200:case 208:case 216:case 224:case 232:case 240:case 248:case 328:case 336:
#define _VERTICAL_SYNC_ case 280:case 281:case 282:case 283:case 284:case 285:case 286:case 287:case 288:case 289:case 290:case 291:case 292:case 293:case 294:case 295:case 296:case 297:case 298:case 299:case 300:case 301:case 302:case 303:case 304:

PPU::PPU(Cart* c,Renderer& r)
:
    ren(r),
    SCROLL_ADDR(*this),
    IO(*this,c),
    plt(*this),
    nmi(*this),
    spr(*this,c),
    bg(*this,c)
{}
uint8_t PPU::read(int addr){
    switch(addr){
        case 2:
            io_latch=STATUS|(io_latch&0x1F);
            SCROLL_ADDR.toggle=false;
            nmi.read();
            break;
        case 4:
            io_latch=spr.read();
            break;
        case 7:
            io_latch=IO.read();
            SCROLL_ADDR.increment();
            break;
    }
    return io_latch;
}
void PPU::write(int addr,uint8_t data){
    switch(addr){
        case 0:
            if(ready){
                nmi.write(data);
                SCROLL_ADDR.write_ctrl(CTRL=data);
                spr.height=CTRL[6]?16:8;
                //if(CTRL[0]&&near the end of a visible scanline)
                //cause a glitch(when vertical or 4-screen mirroring)
                //Bit 0 bus conflict,SCRROLL_ADDR.horizontal_sync(of CTRL_X) will be failed.
            }break;
        case 1:
            if(ready){
                MASK=data;
            }
            break;
        case 3: 
            if(isIdle())
                SPR_ADDR=data;
            //decay $2004(hard to implement)
            break;
        case 4:
            spr.write(data);
            break;
        case 5:
            if(ready){
                SCROLL_ADDR.write_scroll(data);
            }
            break;
        case 6:
            if(ready){
                SCROLL_ADDR.write_addr(data);
            }
            break;
        case 7:
            if(isIdle()){
                IO.write(data);
            }//else 
                //write to unpredictable address
            SCROLL_ADDR.increment();
            break;
    }
    io_latch=data;
}
bool PPU::checkNMI(){
    if( V==241&&(H==0||H==1||H==2) ) 
        return false;
    return nmi.check();
}
void PPU::run(){
    //if ForcedBlank lasts long, OAM begin to decay.
    tick();
    if(V<240){
        render();
    }else switch(V){
        case 240:
            if(H==0){
                ren.update();
                state=IDLE;
            }break;
        case 241:
            if(H==1)
                nmi.set();
            break;
        case 261:
            if(H==0){
                state=BUSY;
            }
            if(H==1){
                STATUS=0;
                spr.copy0xF8();//bug
                spr.hit=false;
                ready=true;
            }
            pre_render();
    }
}
bool PPU::isIdle(){
    return (state==IDLE)||isForcedBlank();
}
bool PPU::isForcedBlank(){
    return !(MASK[4]|MASK[3]);
}
void PPU::tick(){
    if(V==261&&H==339&&frame_parity==ODD&&!isForcedBlank()){
        frame_parity=EVEN,H=0,V=0;
    }else if(++H==341){
        H=0;
        if(++V==262){
            V=0;
            frame_parity=static_cast<Parity>(!frame_parity);
        }
    }
}
void PPU::pre_render(){
    switch(H){
        case 0:
            bg.state=BG::IDLE;
            break;
        case 1:
            bg.state=BG::FETCH;
            break;
        case 256:
            if(!isForcedBlank())
                SCROLL_ADDR.vertical_increment();
                //useless horizontal_increment();
            break;
        case 257:
            //Sprite tile fetches do on the pre-render line.
            spr.state=Sprite::FETCH;
            bg.state=BG::IDLE;
            if(!isForcedBlank())
                SCROLL_ADDR.horizontal_sync();
            break;
        /*_VERTICAL_SYNC_*/case 304: 
            if(!isForcedBlank())
                SCROLL_ADDR.vertical_sync();
            break;
        case 321:
            spr.state=Sprite::IDLE;
            bg.state=BG::FETCH;
            break;
        _HORIZONTAL_INC_
            if(!isForcedBlank())
                SCROLL_ADDR.horizontal_increment();
            break;
    }
    if(!isForcedBlank()){
        spr.drive();
        bg.drive();
    }
}
void PPU::render(){
    switch(H){
        case 0:
            bg.state=BG::IDLE;
            break;
        case 1:
            spr.state=Sprite::CLEAR;
            bg.state=BG::FETCH;
            break;
        case 2:
            if(spr.hit)
                STATUS.setTrue(6);
            break;
        case 65:
            spr.state=Sprite::EVAL;
            break;
        case 256:
            if(!isForcedBlank()){
                SCROLL_ADDR.vertical_increment();
                //useless horizontal_increment();
            }break;
        case 257:
            spr.state=Sprite::FETCH;
            bg.state=BG::IDLE;
            if(!isForcedBlank())
                SCROLL_ADDR.horizontal_sync();
            break;
        case 321:
            spr.state=Sprite::IDLE;
            bg.state=BG::FETCH;
            break;
        case 337:
            bg.state=BG::UNUSED_NT_FETCH;
            break;
        _HORIZONTAL_INC_
            if(!isForcedBlank())
                SCROLL_ADDR.horizontal_increment();
            break;
    }
    if(!isForcedBlank()){
        spr.drive();
        bg.drive();
    }
    if(!(H&0xF00)){//H:0-255
        bool leftmost=!(H&0xF8);
        spr.render(leftmost);
        bg.render(leftmost); 
        mux();
    }
}
void PPU::mux(){
    int index;
    const int i=static_cast<bool>(spr.out.index)<<1|static_cast<bool>(bg.out.index);
    switch(i){
        case 0:
            index=0;
            break;
        case 1:
            index=bg.out.index;
            break;
        case 2:
            index=0x10|spr.out.index;
            break;
        case 3:
            if(spr.out.sprite0&&H!=255)
                spr.hit=true;
            index=spr.out.bg_priority? bg.out.index: 0x10|spr.out.index;
    }
    //pallax effect.
    ren.render(V,H,plt.get(index));
}

void PPU::SCROLL_ADDR::write_ctrl(uint8_t ctrl){
    tmp=( tmp&~(CTRL_Y|CTRL_X) )|(ctrl&0x03)<<10;
}
void PPU::SCROLL_ADDR::write_scroll(uint8_t data){
    if(toggle^=1){
        tmp=(tmp&~SCROLL_X)|data>>3;
        fineX=data&0x07;
    }else{
        tmp=( tmp&~(FINE_Y|SCROLL_Y) ) | (data&0x07)<<12 | (data&0xF8)<<2;
    }
}
void PPU::SCROLL_ADDR::write_addr(uint8_t data){
    if(toggle^=1){
        tmp=(tmp&0x00FF)|((data&0x3F)<<8);
    }else{
        tmp=(tmp&0x7F00)|data;
        address=tmp;
    }
}
void PPU::SCROLL_ADDR::increment(){
    if(p.isIdle()){
        address+=p.CTRL[2]?32:1;
    }else{
        horizontal_increment();
        vertical_increment();
    }
}
void PPU::SCROLL_ADDR::horizontal_sync(){
    address=( address&~(CTRL_X|SCROLL_X) ) | ( tmp&(CTRL_X|SCROLL_X) );
}
void PPU::SCROLL_ADDR::vertical_sync(){
    address=( address&~(FINE_Y|CTRL_Y|SCROLL_Y) ) | ( tmp&(FINE_Y|CTRL_Y|SCROLL_Y) );
}
void PPU::SCROLL_ADDR::horizontal_increment(){
    bool carry=!(~address&SCROLL_X);
    if(carry) 
        address^=CTRL_X|SCROLL_X;
    else
        ++address;
}
void PPU::SCROLL_ADDR::vertical_increment(){
    bool not_carry=~address&FINE_Y;
    if(not_carry){
        address+=0x1000;
    }else switch(address&SCROLL_Y){
        case LIMIT_Y: address^=CTRL_Y;
        case SCROLL_Y:
            address&=~(FINE_Y|SCROLL_Y);
            break;
        default:
            address=(address^FINE_Y)+32;
    }
}
uint16_t PPU::SCROLL_ADDR::get_external_address(){
    return address&BUS_WIDTH;
}
uint16_t PPU::SCROLL_ADDR::get_nametable_address(){
    return NAME_TABLE | ( address&(CTRL_Y|CTRL_X|SCROLL_Y|SCROLL_X) );
}
uint16_t PPU::SCROLL_ADDR::get_colortable_address(){
    return COLOR_TABLE | ( address&(CTRL_Y|CTRL_X) ) | (address&META_BLOCK_Y)>>4 | (address&META_BLOCK_X)>>2;
}
int PPU::SCROLL_ADDR::sellect_pallet_number(uint8_t color_table_data){
    return color_table_data>>( (address&BLOCK_OFSET_Y)>>4 | (address&BLOCK_OFSET_X) );
}
int PPU::SCROLL_ADDR::get_fineX(){
    return fineX;
}
int PPU::SCROLL_ADDR::get_fineY(){
    return (address&FINE_Y)>>12;
}

uint8_t PPU::IO::read(){
    static uint8_t buffer=0;
    uint8_t current=buffer;
    int addr=p.SCROLL_ADDR.get_external_address();
    buffer=cart->chr_read(addr);
    if(~addr&0x3F00){
        return current;
    }else{
        return p.plt.read(addr&0x1F);
    }
}
void PPU::IO::write(uint8_t data){
    int addr=p.SCROLL_ADDR.get_external_address();
    if(~addr&0x3F00){
        cart->chr_write(addr,data);
    }else{
        p.plt.write(addr&0x1F,data);
    }
}

uint8_t PPU::Pallet::read(int index)const{//$00-$1F
    return p.MASK[0]? ram[index]&0x30 : ram[index];
}
void PPU::Pallet::write(int index,uint8_t data){
    ram[index]=data&0x3F;
    if( !(index&0x03) )//$00,$04,$08,$0C,$10,$14,$18,$1C
        ram[index^0x10]=ram[index];
}
int PPU::Pallet::get(int index)const{//$00-$1F
    return (p.MASK&0xE0)<<1 | read(index);
}

void PPU::NMI::read(){
    if(p.V==241&&p.H==0){
        decay=true;
    }
    p.STATUS.setFalse(7);
}
void PPU::NMI::write(uint8_t data){
    if(p.STATUS[7]&&!p.CTRL[7]&&data&0x80){
        occur=true;
    }
}
void PPU::NMI::set(){
    if(!decay){
        p.STATUS.setTrue(7);
        if(p.CTRL[7]){
            occur=true;
        }
    }
    decay=false;
}
bool PPU::NMI::check(){
    if(occur)
        return !(occur=false);
    return false;
}

uint8_t PPU::Sprite::read()const{
    return tmp.latch;
}
void PPU::Sprite::write(uint8_t& data){
    if(p.isIdle()){
        if((p.SPR_ADDR&0x03)==2)
            data&=0xE3;
        ram[p.SPR_ADDR++]=data;
    }else{
        p.SPR_ADDR+=4;
    }
}
void PPU::Sprite::copy0xF8(){//reproduce bug
    const int index=p.SPR_ADDR&0xF8; 
    if(index) 
       memcpy(ram,ram+index,8);
}
void PPU::Sprite::drive(){
    switch(state){
        case IDLE:
            idle();
            break;
        case CLEAR:
            if(p.H==1)
                tmp.clear();
            break;
        case EVAL:
            eval();
            break;
        case FETCH:
            fetch();
    }
}
void PPU::Sprite::idle(){
    tmp.latch=tmp.ram[0];
}
void PPU::Sprite::eval(){
    switch(p.H&1){
        case ODD://こちらからはじまる
            tmp.latch = ram[p.SPR_ADDR];
            break;
        case EVEN:
            if(tmp.all_spr_evaluated){
                tmp.write(false);
                add4_SPR_ADDR();
                break;
            }
            int offset = tmp.index & 0x03;
            if(!offset){
                const int diff=p.V-tmp.latch;
                if( 0<= diff && diff < height){
                    if(!tmp.write_enable)
                        p.STATUS.setTrue(5);
                    tmp.write(true);
                    inc_SPR_ADDR();
                }else if(!tmp.write_enable&!p.STATUS[5]){//hardware bug
                    tmp.write(false);
                    dual_inc_SPR_ADDR();
                }else{
                    tmp.write(false);
                    add4_SPR_ADDR();
                }
            }else{
                tmp.write(true);
                inc_SPR_ADDR();
            }
    }
}
void PPU::Sprite::inc_SPR_ADDR(){
    if(!++p.SPR_ADDR)
        tmp.all_spr_evaluated=true;
}
void PPU::Sprite::add4_SPR_ADDR(){
    p.SPR_ADDR+=4;
    if(!(p.SPR_ADDR&0xFC))
        tmp.all_spr_evaluated=true;
}
void PPU::Sprite::dual_inc_SPR_ADDR(){
    if(++p.SPR_ADDR&0x03){
        p.SPR_ADDR+=4;
    }
    if(!(p.SPR_ADDR&0xFC))
        tmp.all_spr_evaluated=true;
}
void PPU::Sprite::fetch(){
    p.SPR_ADDR=0;
    static int eff_spr;
    static bool dummy;
    static int count;
    static uint8_t y_coordinate;
    static uint8_t tile_num;
    static bool V_flip;
    static bool H_flip;
    if(p.H==257){
        dummy=false;
        eff_spr=tmp.write_enable?tmp.index>>2:8;
        count=tmp.index=0;
    }
    switch(p.H&0x07){
        case 1:
            if(count==eff_spr)
                dummy=true;
            y_coordinate=tmp.read();
            break;
        case 2:
            tile_num=tmp.read();
            // p.bg.unused_NT_fetch();
            break;
        case 3:{
            Uint8_t attr=tmp.read();
            V_flip=attr[7];
            H_flip=attr[6];
            buf[count].attributes=attr;
            }break;
        case 4:
            buf[count].H_position=tmp.read();
            // p.bg.unused_NT_fetch();
            break;
        case 5:
            break;
        case 6:
            buf[count].pattern_low=dummy?0:tile_load(tile_num,false,p.V-y_coordinate,V_flip,H_flip);
            break;
        case 7:
            break;
        case 0:
            buf[count++].pattern_hi=dummy?0:tile_load(tile_num,true,p.V-y_coordinate,V_flip,H_flip);
            break;
    }
}
uint8_t PPU::Sprite::tile_load(uint8_t tile_num,bool upper,int Y_offset,bool V_flip,bool H_flip){
    uint16_t addr;
    if(V_flip){
        Y_offset=~Y_offset;
    }
    if(p.CTRL[5]){//8 16 mode
         addr=(tile_num&0x01)<<12 | (tile_num&0xFE)<<4 | (Y_offset&0x08)<<1 | upper<<3 | (Y_offset&0x07);
    }else{
        addr=p.CTRL[3]<<12 | tile_num<<4 | upper<<3 | (Y_offset&0x07);
    }
    uint8_t data=cart->chr_read(addr);
    if(H_flip){
        uint8_t tmp=0;
        for (int i = 0; i < 8; ++i){
            tmp = (tmp<<1) | (data&0x01);
            data>>=1;
        }
        data=tmp;
    }
    return data;
}
void PPU::Sprite::Temporary_Memory::clear(){
    all_spr_evaluated=false;
    write_enable=true;
    latch=0xFF;
    memset(ram,latch,size);
    index=0;
}
void PPU::Sprite::Temporary_Memory::write(bool inc){
    if(write_enable&&!all_spr_evaluated){
        ram[index]=latch;
    }else{
        latch=write_enable?ram[index]:ram[0];
    }
    if(inc&&++index==32){
        write_enable=false;
        index=0;
    }
}
uint8_t PPU::Sprite::Temporary_Memory::read(){
    return latch=ram[index++];
}
void PPU::Sprite::render(bool leftmost){
    out.index=0;
    bool show=p.MASK[4]&& ( !leftmost||p.MASK[2] );
    for (int i=0;i<8;++i){
        if(!buf[i].H_position){
            if(show&&!out.index){//detect first opaque spr
                int col_num=(buf[i].pattern_hi&0x80)>>6|buf[i].pattern_low>>7;
                if(col_num){//non-opaque
                    out.index=(buf[i].attributes&0x03)<<2 | col_num;
                    out.sprite0=!i;
                    out.bg_priority=buf[i].attributes&0x20;
                }
            }
            buf[i].pattern_low<<=1;
            buf[i].pattern_hi<<=1;
        }else{
            --buf[i].H_position;
        }
    }
}

void PPU::BG::drive(){
    switch(state){
        case IDLE:
            break;
        case FETCH:
            fetch();
            break;
        case UNUSED_NT_FETCH:
            break;
            // unused_NT_fetch();
    }
}
void PPU::BG::fetch(){
    buf.shift();//in real nes H:2-257,322-337
    static uint8_t tile_num;
    switch(p.H&0x07){
        case 1://74LS373
            break;
        case 2:
            tile_num=cart->chr_read(p.SCROLL_ADDR.get_nametable_address());
            break;
        case 3:
            break;
        case 4:{
            uint8_t color_table_data=cart->chr_read(p.SCROLL_ADDR.get_colortable_address());
            int s=p.SCROLL_ADDR.sellect_pallet_number(color_table_data);
            buf.pallet_low_tmp=s&0x01;
            buf.pallet_hi_tmp=s&0x02;
            }break;
        case 5:
            break;
        case 6:
            buf.pattern_low_tmp=tile_load(tile_num,false);
            break;
        case 7:
            break;
        case 0:
            buf.pattern_hi_tmp=tile_load(tile_num,true);
            buf.load();//in real nes H:9,17,25...257
            break;
    }

}
void PPU::BG::unused_NT_fetch(){
    switch(p.H&0x01){
        case 1:
            break;
        case 0:
            cart->chr_read(p.SCROLL_ADDR.get_nametable_address());
    }
}
uint8_t PPU::BG::tile_load(uint8_t tile_num,bool upper){
    uint16_t addr=p.CTRL[4]<<12 | tile_num<<4 | upper<<3 | p.SCROLL_ADDR.get_fineY();
    return cart->chr_read(addr);
}
void PPU::BG::render(bool leftmost){
    bool show=p.MASK[3]&& ( !leftmost||p.MASK[1] );
    if(show){
        uint16_t eff_bit=0x8000>>p.SCROLL_ADDR.get_fineX();
        int col_num=static_cast<bool>(buf.pattern_hi&eff_bit)<<1|static_cast<bool>(buf.pattern_low&eff_bit);
        if(!col_num)
            goto Opaque;
        int pal_num=static_cast<bool>(buf.pallet_hi&eff_bit)<<3|static_cast<bool>(buf.pallet_low&eff_bit)<<2;
        out.index=pal_num|col_num;
    }else{
    Opaque:
        out.index=0;
    }
}
void PPU::BG::Buffer::load(){
    pattern_low|=pattern_low_tmp;
    pattern_hi|=pattern_hi_tmp;
    pallet_low|=pallet_low_tmp?0xFF:0;
    pallet_hi|=pallet_hi_tmp?0xFF:0;
}
void PPU::BG::Buffer::shift(){
    pattern_low<<=1;
    pattern_hi<<=1;
    pallet_low<<=1;
    pallet_hi<<=1;
}