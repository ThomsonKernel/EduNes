#pragma once
#include "Uint8_t.h"
#include "SDL_Video.h"

class Cart;

class PPU{
public:
    PPU(Cart*,Renderer&);
    uint8_t read(int addr);
    void write(int addr,uint8_t data); 
    void run();
    bool checkNMI();
private:
    Renderer& ren;

    bool isForcedBlank();
    bool isIdle();
    void tick();
    void pre_render();
    void render();
    void mux();

    uint8_t io_latch;
    int V=0;
    int H=-1;
    enum Parity{
        EVEN,ODD
    }frame_parity=EVEN;
    bool ready=false;//internal_reset_signal
    enum{
        IDLE,BUSY
    }state=BUSY;

    //Registers
    Uint8_t CTRL=0, MASK=0, STATUS=0;
    uint8_t SPR_ADDR=0;
    struct SCROLL_ADDR{
        PPU& p;
        SCROLL_ADDR(PPU& p):p(p){};
        bool toggle=false;
        void write_ctrl(uint8_t);
        void write_scroll(uint8_t);
        void write_addr(uint8_t);
        void increment();
        void horizontal_sync();
        void vertical_sync();
        void horizontal_increment();
        void vertical_increment();
        uint16_t get_external_address();
        uint16_t get_nametable_address();
        uint16_t get_colortable_address();
        int sellect_pallet_number(uint8_t color_table_data);
        int get_fineX();
        int get_fineY();
    private:
        enum{
            FINE_Y=0x7000,
            CTRL_Y=0x0800,
            CTRL_X=0x0400,
            SCROLL_Y=0x03E0,
            SCROLL_X=0x001F,
            LIMIT_Y=0x03A0,
            BUS_WIDTH=0x3FFF,
            NAME_TABLE=0x2000,
            COLOR_TABLE=0x23C0,
            META_BLOCK_Y=0x0380,
            META_BLOCK_X=0x001C,
            BLOCK_OFSET_Y=0x0040,
            BLOCK_OFSET_X=0x0002
        };
        uint16_t tmp:15;
        uint16_t address:15;
        uint8_t fineX:3;
    }SCROLL_ADDR;
    struct IO{
        PPU& p;
        Cart* cart;
        explicit IO(PPU& p,Cart* c):p(p),cart(c){};
        uint8_t read();
        void write(uint8_t data);
    }IO;
    ///////////////////
    struct Pallet{
        PPU& p;
        explicit Pallet(PPU& p):p(p){};
        uint8_t read(int index)const;
        void write(int index,uint8_t data);
        int get(int index)const;
    private:
        uint8_t ram[0x20];
    }plt;
    struct NMI{
        PPU& p;
        explicit NMI(PPU& p):p(p){};
        void read();        //$2002(R)
        void write(uint8_t);//$2000(W)
        void set();
        bool check();
    private:
        bool occur=false;
        bool decay=false;
    }nmi;
    struct Sprite{
        PPU& p;
        Cart* cart;
        explicit Sprite(PPU& p,Cart* c):p(p),cart(c){};
        uint8_t read()const;//$2004(R)
        void write(uint8_t& data);//$2004(W)
        enum{
            IDLE,CLEAR,EVAL,FETCH
        }state=IDLE;
        int height=8;
        bool hit;//for delay
        void copy0xF8();
        void drive();
        void render(bool leftmost);
        struct Output{
            bool sprite0;
            bool bg_priority;
            unsigned int index:4;
        }out;
    private:
        uint8_t ram[0x100];
        void idle();
        void eval();
        void fetch();
        uint8_t tile_load(uint8_t tile_num,bool upper,int Y_offset,bool V_flip,bool H_flip);
        void inc_SPR_ADDR();
        void add4_SPR_ADDR();
        void dual_inc_SPR_ADDR();
        struct Temporary_Memory{
            static constexpr int size=0x20;
            uint8_t ram[size];
            int index;
            bool all_spr_evaluated;//SPR_ADDR overflow
            bool write_enable;//OAM pointer overflow
            uint8_t latch;
            void clear();
            void write(bool inc);
            uint8_t read();
        }tmp;
        struct Buffer{
            uint8_t pattern_low;//shift_register
            uint8_t pattern_hi;//shift_register
            uint8_t attributes;
            uint8_t H_position;//down counter
        }buf[8];
    }spr;

    struct BG{
        PPU &p;
        Cart* cart;
        explicit BG(PPU& p,Cart* c):p(p),cart(c){};
        enum{
            IDLE,FETCH,UNUSED_NT_FETCH
        }state=IDLE;
        void drive();
        void render(bool leftmost);
        struct Output{
            unsigned int index:4;//0:opaque
        }out;
        void unused_NT_fetch();
    private:
        void fetch();
        struct Buffer{
            uint16_t pattern_low;//8bits*2(shift_register)
            uint16_t pattern_hi;
            uint16_t pallet_low;//8bits(shift_register)+1bit latch
            uint16_t pallet_hi;
            //tmp
            uint8_t pattern_low_tmp;
            uint8_t pattern_hi_tmp;
            bool pallet_low_tmp;
            bool pallet_hi_tmp;
            void load();
            void shift();
        }buf;
        uint8_t tile_load(uint8_t tile_num,bool upper);
        uint8_t color_load();
    }bg;
};