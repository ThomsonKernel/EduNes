#include <stdint.h>
#include "Cart.h"
#include "PPU.h"
#include "RAM.h"
#include "SDL_Input.h"
#include "CPU.h"

CPU::CPU(Cart* c,PPU& p)
:
    cart(c),
    ppu(p)
{
    ram=new RAM;
    input=new Input;

    setPSR(0x34);
    A=X=Y=0;
    SP=0xFD;

    PC=static_cast<uint16_t>( read(0xFFFD)<<8|read(0xFFFC) );
}
CPU::~CPU(){
    delete ram;
    delete input;
}
void CPU::run(){
    if(cyc.shouldExec()){
        if(ppu.checkNMI()){
            NMI();
        }else if(dma_running){
            dma_exec();
        }else{
            exec();
        }
    }
}
uint8_t CPU::read(uint16_t addr){
    switch(addr&0xE000){
        case 0x0000:
            return ram->read(addr&0x07FF);
        case 0x2000:
            return ppu.read(addr&7);
        case 0x4000:
            if(!(addr&0x10E0)){//$4000-$401F
                switch(addr){
                    case 0x4014://dma
                        return 0;
                    case 0x4016://joy1
                        return input->read();
                    case 0x4017://joy2
                        return 0;
                    default://APU
                        return 0;
                }
            }
        default:
            return cart->prg_read(addr);
    }
}
void CPU::write(uint16_t addr,uint8_t data){
    switch(addr & 0xE000){
        case 0x0000:
            ram->write(addr&0x07FF,data);
            break;
        case 0x2000:
            ppu.write(addr&7,data);
            break;
        case 0x4000:
            if(!(addr&0x10E0)){
                switch(addr){
                    case 0x4014://dma
                        dma_start(data);
                        break;
                    case 0x4016://joy1
                        input->write(data);
                        break;
                    case 0x4017://joy2
                        break;;
                    default://APU
                        break;;
                }
            }
        default:
            cart->prg_write(addr,data);
    }
}
void CPU::exec(){
    uint8_t opcode=read(PC++);
    cyc.init(opcode);
    switch(opcode){
        case 0xa9: LDA(immediate());           break;
        case 0xa5: LDA(zeropage());            break;
        case 0xad: LDA(absolute());            break;
        case 0xb5: LDA(zeropage_x());          break;
        case 0xbd: LDA(absolute_x());          break;
        case 0xb9: LDA(absolute_y());          break;
        case 0xa1: LDA(zeropage_x_indirect()); break;
        case 0xb1: LDA(zeropage_indirect_y()); break;

        case 0xa2: LDX(immediate());           break;
        case 0xa6: LDX(zeropage());            break;
        case 0xae: LDX(absolute());            break;
        case 0xb6: LDX(zeropage_y());          break;
        case 0xbe: LDX(absolute_y());          break;

        case 0xa0: LDY(immediate());           break;
        case 0xa4: LDY(zeropage());            break;
        case 0xac: LDY(absolute());            break;
        case 0xb4: LDY(zeropage_x());          break;
        case 0xbc: LDY(absolute_x());          break;

        case 0x85: STA(zeropage());            break;
        case 0x8d: STA(absolute());            break;
        case 0x95: STA(zeropage_x());          break;
        case 0x9d: STA(absolute_x());          break;
        case 0x99: STA(absolute_y());          break;
        case 0x81: STA(zeropage_x_indirect()); break;
        case 0x91: STA(zeropage_indirect_y()); break;

        case 0x86: STX(zeropage());            break;
        case 0x8e: STX(absolute());            break;
        case 0x96: STX(zeropage_y());          break;

        case 0x84: STY(zeropage());            break;
        case 0x8c: STY(absolute());            break;
        case 0x94: STY(zeropage_x());          break;

        case 0x69: ADC(immediate());           break;
        case 0x65: ADC(zeropage());            break;
        case 0x6d: ADC(absolute());            break;
        case 0x75: ADC(zeropage_x());          break;
        case 0x7d: ADC(absolute_x());          break;
        case 0x79: ADC(absolute_y());          break;
        case 0x61: ADC(zeropage_x_indirect()); break;
        case 0x71: ADC(zeropage_indirect_y()); break;

        case 0xe9: SBC(immediate());           break;
        case 0xe5: SBC(zeropage());            break;
        case 0xed: SBC(absolute());            break;
        case 0xf5: SBC(zeropage_x());          break;
        case 0xfd: SBC(absolute_x());          break;
        case 0xf9: SBC(absolute_y());          break;
        case 0xe1: SBC(zeropage_x_indirect()); break;
        case 0xf1: SBC(zeropage_indirect_y()); break;

        case 0xe0: CPX(immediate());           break;
        case 0xe4: CPX(zeropage());            break;
        case 0xec: CPX(absolute());            break;

        case 0xc0: CPY(immediate());           break;
        case 0xc4: CPY(zeropage());            break;
        case 0xcc: CPY(absolute());            break;

        case 0xc9: CMP(immediate());           break;
        case 0xc5: CMP(zeropage());            break;
        case 0xcd: CMP(absolute());            break;
        case 0xd5: CMP(zeropage_x());          break;
        case 0xdd: CMP(absolute_x());          break;
        case 0xd9: CMP(absolute_y());          break;
        case 0xc1: CMP(zeropage_x_indirect()); break;
        case 0xd1: CMP(zeropage_indirect_y()); break;
        
        case 0x29: AND(immediate());           break;
        case 0x25: AND(zeropage());            break;
        case 0x2d: AND(absolute());            break;
        case 0x35: AND(zeropage_x());          break;
        case 0x3d: AND(absolute_x());          break;
        case 0x39: AND(absolute_y());          break;
        case 0x21: AND(zeropage_x_indirect()); break;
        case 0x31: AND(zeropage_indirect_y()); break;

        case 0x49: EOR(immediate());           break;
        case 0x45: EOR(zeropage());            break;
        case 0x4d: EOR(absolute());            break;
        case 0x55: EOR(zeropage_x());          break;
        case 0x5d: EOR(absolute_x());          break;
        case 0x59: EOR(absolute_y());          break;
        case 0x41: EOR(zeropage_x_indirect()); break;
        case 0x51: EOR(zeropage_indirect_y()); break;

        case 0x09: ORA(immediate());           break;
        case 0x05: ORA(zeropage());            break;
        case 0x0d: ORA(absolute());            break;
        case 0x15: ORA(zeropage_x());          break;
        case 0x1d: ORA(absolute_x());          break;
        case 0x19: ORA(absolute_y());          break;
        case 0x01: ORA(zeropage_x_indirect()); break;
        case 0x11: ORA(zeropage_indirect_y()); break;

        case 0x24: BIT(zeropage());            break;
        case 0x2c: BIT(absolute());            break;

        case 0x0a: ASL();                      break;
        case 0x06: ASL(zeropage());            break;
        case 0x0e: ASL(absolute());            break;
        case 0x16: ASL(zeropage_x());          break;
        case 0x1e: ASL(absolute_x());          break;

        case 0x4a: LSR();                      break;
        case 0x46: LSR(zeropage());            break;
        case 0x4e: LSR(absolute());            break;
        case 0x56: LSR(zeropage_x());          break;
        case 0x5e: LSR(absolute_x());          break;

        case 0x2a: ROL();                      break;
        case 0x26: ROL(zeropage());            break;
        case 0x2e: ROL(absolute());            break;
        case 0x36: ROL(zeropage_x());          break;
        case 0x3e: ROL(absolute_x());          break;

        case 0x6a: ROR();                      break;
        case 0x66: ROR(zeropage());            break;
        case 0x6e: ROR(absolute());            break;
        case 0x76: ROR(zeropage_x());          break;
        case 0x7e: ROR(absolute_x());          break;

        case 0xe6: INC(zeropage());            break;
        case 0xee: INC(absolute());            break;
        case 0xf6: INC(zeropage_x());          break;
        case 0xfe: INC(absolute_x());          break;

        case 0xc6: DEC(zeropage());            break;
        case 0xce: DEC(absolute());            break;
        case 0xd6: DEC(zeropage_x());          break;
        case 0xde: DEC(absolute_x());          break;

        case 0xea: NOP();                      break;
        case 0x00: BRK();                      break;
        case 0x40: RTI();                      break;
        case 0x60: RTS();                      break;
        case 0x20: JSR(absolute());            break;
        case 0x4c: JMP(absolute());            break;
        case 0x6c: JMP(absolute_indirect());   break;

        case 0x10: BPL();                      break;
        case 0x30: BMI();                      break;
        case 0x50: BVC();                      break;
        case 0x70: BVS();                      break;
        case 0x90: BCC();                      break;
        case 0xB0: BCS();                      break;
        case 0xD0: BNE();                      break;
        case 0xF0: BEQ();                      break;

        case 0x18: CLC();                      break;
        case 0x38: SEC();                      break;
        case 0x58: CLI();                      break;
        case 0x78: SEI();                      break;
        case 0xb8: CLV();                      break;
        case 0xd8: CLD();                      break;
        case 0xf8: SED();                      break;

        case 0xaa: TAX();                      break;
        case 0x8a: TXA();                      break;
        case 0xca: DEX();                      break;
        case 0xe8: INX();                      break;
        case 0xa8: TAY();                      break;
        case 0x98: TYA();                      break;
        case 0x88: DEY();                      break;
        case 0xc8: INY();                      break;

        case 0x9a: TXS();                      break;
        case 0xba: TSX();                      break;
        case 0x48: PHA();                      break;
        case 0x68: PLA();                      break;
        case 0x08: PHP();                      break;
        case 0x28: PLP();                      break;

        //Unofficial opcodes
        case 0x04:case 0x44:case 0x64: 
            _NOP(zeropage());
          break;
        case 0x0c:
            _NOP(absolute());
          break;
        case 0x14:case 0x34:case 0x54:case 0x74:case 0xD4:case 0xF4:
            _NOP(zeropage_x_indirect());
          break;
        case 0x1A:case 0x3A:case 0x5A:case 0x7A:case 0xDA:case 0xFA:
            _NOP();
          break;
        case 0x80:case 0x82:case 0x89:case 0xC2:case 0xE2:
            _NOP(immediate());
          break;
        case 0x1C:case 0x3C:case 0x5C:case 0x7C:case 0xDC:case 0xFC:
            _NOP(absolute_x());
          break;

        case 0x07: _SLO(zeropage());            break;
        case 0x17: _SLO(zeropage_x());          break;
        case 0x03: _SLO(zeropage_x_indirect()); break;
        case 0x13: _SLO(zeropage_indirect_y()); break;
        case 0x0F: _SLO(absolute());            break;
        case 0x1F: _SLO(absolute_x());          break;
        case 0x1B: _SLO(absolute_y());          break;

        case 0x27: _RLA(zeropage());            break;
        case 0x37: _RLA(zeropage_x());          break;
        case 0x23: _RLA(zeropage_x_indirect()); break;
        case 0x33: _RLA(zeropage_indirect_y()); break;
        case 0x2F: _RLA(absolute());            break;
        case 0x3F: _RLA(absolute_x());          break;
        case 0x3B: _RLA(absolute_y());          break;

        case 0x47: _SRE(zeropage());            break;
        case 0x57: _SRE(zeropage_x());          break;
        case 0x43: _SRE(zeropage_x_indirect()); break;
        case 0x53: _SRE(zeropage_indirect_y()); break;
        case 0x4F: _SRE(absolute());            break;
        case 0x5F: _SRE(absolute_x());          break;
        case 0x5B: _SRE(absolute_y());          break;

        case 0x67: _RRA(zeropage());            break;
        case 0x77: _RRA(zeropage_x());          break;
        case 0x63: _RRA(zeropage_x_indirect()); break;
        case 0x73: _RRA(zeropage_indirect_y()); break;
        case 0x6F: _RRA(absolute());            break;
        case 0x7F: _RRA(absolute_x());          break;
        case 0x7B: _RRA(absolute_y());          break;

        case 0x87: _SAX(zeropage());            break;
        case 0x97: _SAX(zeropage_y());          break;
        case 0x83: _SAX(zeropage_x_indirect()); break;
        case 0x8F: _SAX(absolute());            break;

        case 0xA7: _LAX(zeropage());            break;
        case 0xB7: _LAX(zeropage_y());          break;
        case 0xA3: _LAX(zeropage_x_indirect()); break;
        case 0xB3: _LAX(zeropage_indirect_y()); break;
        case 0xAF: _LAX(absolute());            break;
        case 0xBF: _LAX(absolute_y());          break;

        case 0xC7: _DCP(zeropage());            break;
        case 0xD7: _DCP(zeropage_x());          break;
        case 0xC3: _DCP(zeropage_x_indirect()); break;
        case 0xD3: _DCP(zeropage_indirect_y()); break;
        case 0xCF: _DCP(absolute());            break;
        case 0xDF: _DCP(absolute_x());          break;
        case 0xDB: _DCP(absolute_y());          break;

        case 0xE7: _ISC(zeropage());            break;
        case 0xF7: _ISC(zeropage_x());          break;
        case 0xE3: _ISC(zeropage_x_indirect()); break;
        case 0xF3: _ISC(zeropage_indirect_y()); break;
        case 0xEF: _ISC(absolute());            break;
        case 0xFF: _ISC(absolute_x());          break;
        case 0xFB: _ISC(absolute_y());          break;

        case 0x0B:case 0x2B: _ANC(immediate()); break;
        case 0x4B: _ALR(immediate());           break;
        case 0x6B: _ARR(immediate());           break;
        case 0x8B: _XAA(immediate());           break;
        case 0xAB: _LAXi(immediate());          break;
        case 0xCB: _AXS(immediate());           break;
        case 0xEB: _SBC(immediate());           break;

        case 0x93: _AHX(zeropage_indirect_y()); break;
        case 0x9F: _AHX(absolute_y());          break;

        case 0x9C: _SHY(absolute_x());          break;
        case 0x9E: _SHX(absolute_y());          break;
        case 0x9B: _TAS(absolute_y());          break;
        case 0xBB: _LAS(absolute_y());          break;

        case 0x02:case 0x12:case 0x22:case 0x32:case 0x42:case 0x52:
        case 0x62:case 0x72:case 0x92:case 0xB2:case 0xD2:case 0xF2:
            throw "KIL CPU";
        }
}
void CPU::NMI(){
    cyc.nmi();
    push(PC>>8);
    push(PC&0xFF);
    push(PSR_to_uint8_t());
    I=true;
    PC=static_cast<uint16_t>( read(0xFFFB)<<8|read(0xFFFA) );
}
void CPU::IRQ(){
    if(I)
        return;
    push(PC>>8);
    push(PC&0xFF);
    push(PSR_to_uint8_t());
    I=true;
    PC=static_cast<uint16_t>( read(0xFFFF)<<8|read(0xFFFE) );
    return;
}
constexpr int CPU::Cycle::cycle_table[0x100];
constexpr bool CPU::Cycle::penalty_table[0x100];
bool CPU::Cycle::shouldExec(){
    if(--cycle)
        return false;
    return true;
}
void CPU::Cycle::init(uint8_t opcode){
    cycle=cycle_table[opcode];
    penalty=penalty_table[opcode];
}
void CPU::Cycle::nmi(){
    cycle=7;
}
void CPU::Cycle::dma(){
    ++cycle;
}
void CPU::Cycle::branch(){
    ++cycle;
}
uint16_t CPU::Cycle::add(uint8_t a,uint8_t b){
    uint16_t result=a+b;
    if(penalty&&result&0x0100)
        ++cycle;
    return result;
}
uint16_t CPU::Cycle::relative(uint16_t pc,int8_t offset){
    int result=pc+offset;//signed(-128...127)
    if((pc^result)&0x0100)
        ++cycle;
    return static_cast<uint16_t>(result);
}

void CPU::push(uint8_t data){ 
    write(0x0100|SP--,data);
}
uint8_t CPU::pop(){ 
    return read(0x0100|++SP); 
}

uint16_t CPU::immediate(){
    read(PC);//
    return PC++;
}
uint16_t CPU::zeropage(){
    return read(PC++)&0xFF;
}
uint16_t CPU::absolute(){
    uint8_t lower=read(PC++);
    uint8_t upper=read(PC++);
    return static_cast<uint16_t>(upper<<8|lower);
}
uint16_t CPU::zeropage_x(){
    return read(PC++)+X & 0xFF;
}
uint16_t CPU::zeropage_y(){
    return read(PC++)+Y & 0xFF;
}
uint16_t CPU::absolute_x(){
    uint8_t lower=read(PC++);
    uint8_t upper=read(PC++);
    return static_cast<uint16_t>( (upper<<8) + cyc.add(lower,X) );
}
uint16_t CPU::absolute_y(){
    uint8_t lower=read(PC++);
    uint8_t upper=read(PC++);
    return static_cast<uint16_t>( (upper<<8) + cyc.add(lower,Y) );
}
uint16_t CPU::zeropage_x_indirect(){
    uint8_t tmp=static_cast<uint8_t>(zeropage_x());
    uint8_t lower=read(tmp++);
    uint8_t upper=read(tmp);
    return static_cast<uint16_t>( upper<<8|lower );
}
uint16_t CPU::zeropage_indirect_y(){
    uint8_t tmp=static_cast<uint8_t>(zeropage());
    uint8_t lower=read(tmp++);
    uint8_t upper=read(tmp);
    return static_cast<uint16_t>( (upper<<8) + cyc.add(lower,Y) );
}
uint16_t CPU::absolute_indirect(){
    uint16_t tmp=absolute();
    uint8_t lower=read(tmp);
    (~tmp&0xFF)?++tmp:tmp^=0xFF;
    uint8_t upper=read(tmp);
    return static_cast<uint16_t>( upper<<8|lower );
}
uint16_t CPU::relative(){
    int8_t offset=static_cast<int8_t>(read(PC++));
    return cyc.relative(PC,offset);
}

void CPU::LDA(uint16_t addr){
    A=read(addr);
    updateNZ(A);
}
void CPU::LDX(uint16_t addr){
    X=read(addr);
    updateNZ(X);
}
void CPU::LDY(uint16_t addr){
    Y=read(addr);
    updateNZ(Y);
}
void CPU::STA(uint16_t addr){ 
    write(addr,A);
}
void CPU::STX(uint16_t addr){ 
    write(addr,X);
}
void CPU::STY(uint16_t addr){ 
    write(addr,Y);
}
void CPU::ADC(uint16_t addr){
    uint8_t operand=read(addr);
    int result=A+operand+C;
    updateV(operand,result);
    A=updateC(result);
    updateNZ(A);
}
void CPU::SBC(uint16_t addr){
    uint8_t operand=read(addr);
    int result=A+static_cast<uint8_t>(~operand)+C;
    updateV(~operand,result);
    A=updateC(result);
    updateNZ(A);
}
void CPU::CPX(uint16_t addr){
    uint8_t operand=read(addr);
    int result=X+static_cast<uint8_t>(~operand)+1;
    updateNZ(updateC(result));
}
void CPU::CPY(uint16_t addr){
    uint8_t operand=read(addr);
    int result=Y+static_cast<uint8_t>(~operand)+1;
    updateNZ(updateC(result));
}
void CPU::CMP(uint16_t addr){
    uint8_t operand=read(addr);
    int result=A+static_cast<uint8_t>(~operand)+1;
    updateNZ(updateC(result));
}
void CPU::AND(uint16_t addr){
    A&=read(addr);
    updateNZ(A);
}
void CPU::EOR(uint16_t addr){
    A^=read(addr);
    updateNZ(A);
}
void CPU::ORA(uint16_t addr){
    A|=read(addr);
    updateNZ(A);
}
void CPU::BIT(uint16_t addr){
    uint8_t operand=read(addr);
    Z=!(A&operand);
    N=operand&0x80;
    V=operand&0x40;
}
void CPU::ASL(){
    A=updateC(A<<1);
    updateNZ(A);
}
void CPU::ASL(uint16_t addr){
    uint8_t result=updateC(read(addr)<<1);
    updateNZ(result);
    write(addr,result);
}
void CPU::LSR(){
    C=A&0x01;
    A>>=1;
    updateNZ(A);
}
void CPU::LSR(uint16_t addr){
    uint8_t operand=read(addr);
    C=operand&0x01;
    uint8_t result=operand>>1;
    updateNZ(result);
    write(addr,result);
}
void CPU::ROL(){
    A=updateC(A<<1|C);
    updateNZ(A);
}
void CPU::ROL(uint16_t addr){
    uint8_t operand=read(addr);
    uint8_t result=updateC(operand<<1|C);
    updateNZ(result);
    write(addr,result);
}
void CPU::ROR(){
    A=updateC(A<<8|C<<7|A>>1);
    updateNZ(A);
}
void CPU::ROR(uint16_t addr){
    uint8_t operand=read(addr);
    uint8_t result=updateC(operand<<8|C<<7|operand>>1);
    updateNZ(result);
    write(addr,result);
}
void CPU::INC(uint16_t addr){
    uint8_t result=read(addr)+1;
    updateNZ(result);
    write(addr,result);
}
void CPU::DEC(uint16_t addr){
    uint8_t result=read(addr)-1;
    updateNZ(result);
    write(addr,result);
}
void CPU::NOP(){
}
void CPU::BRK(){
    push(++PC>>8);
    push(PC&0xFF);
    push(PSR_to_uint8_t()|B);
    I=true;
    PC=static_cast<uint16_t>(read(0xFFFF)<<8|read(0xFFFE));
}
void CPU::RTI(){
    setPSR(pop());
    uint8_t lower=pop();
    uint8_t upper=pop();
    PC=static_cast<uint16_t>(upper<<8|lower);
}
void CPU::RTS(){
    uint8_t lower = pop();
    uint8_t upper = pop();
    PC=static_cast<uint16_t>( (upper<<8|lower)+1 );
}
void CPU::JSR(uint16_t addr){
    push(--PC>>8);
    push(static_cast<uint8_t>(PC));
    PC=addr;
}
void CPU::JMP(uint16_t addr){
    PC=addr;
}

void CPU::branch(bool b){
    if(b){
        PC=relative();
        cyc.branch();
    }else{
        immediate();
    }
}
void CPU::BPL(){
    branch(!N);
}
void CPU::BMI(){
    branch(N);
}
void CPU::BVC(){
    branch(!V);
}
void CPU::BVS(){
    branch(V);
}
void CPU::BCC(){
    branch(!C);
}
void CPU::BCS(){
    branch(C);
}
void CPU::BNE(){
    branch(!Z);
}
void CPU::BEQ(){
    branch(Z);
}
void CPU::CLC(){
    C=false;
}
void CPU::SEC(){
    C=true;
}
void CPU::CLI(){
    I=false;
}
void CPU::SEI(){
    I=true;
}
void CPU::CLV(){
    V=false;
}
void CPU::CLD(){
    D=false;
}
void CPU::SED(){
    D=true;
}
void CPU::TAX(){
    X=A;
    updateNZ(X);
}
void CPU::TXA(){
    A=X;
    updateNZ(A);
}
void CPU::DEX(){
    X--;
    updateNZ(X);
}
void CPU::INX(){
    X++;
    updateNZ(X);
}
void CPU::TAY(){
    Y=A;
    updateNZ(Y);
}
void CPU::TYA(){
    A=Y;
    updateNZ(Y);
}
void CPU::DEY(){
    Y--;
    updateNZ(Y);
}
void CPU::INY(){
    Y++;
    updateNZ(Y);
}
void CPU::TXS(){
    SP=X;
}
void CPU::TSX(){
    X=SP;
    updateNZ(X);
}
void CPU::PHA(){
    push(A);
}
void CPU::PLA(){
    A=pop();
    updateNZ(A);
}
void CPU::PHP(){
    push(PSR_to_uint8_t()|B);
}
void CPU::PLP(){
    setPSR(pop());
}
void CPU::updateNZ(uint8_t operand){
    N=operand&0x80;
    Z=!operand;
}
void CPU::updateV(int operand,int result){
    V= ~(A^operand)&(A^result) & 0x80;
}
uint8_t CPU::updateC(int operand){
    C=operand&0x0100;
    return static_cast<uint8_t>(operand);
}

void CPU::setPSR(uint8_t data){
    N=0x80&data;
    V=0x40&data;
    D=0x08&data;
    I=0x04&data;
    Z=0x02&data;
    C=0x01&data;
}
uint8_t CPU::PSR_to_uint8_t(){
    return static_cast<uint8_t>(N<<7 | V<<6 | R | D<<3 | I<<2 | Z<<1 | C);
}

void CPU::_NOP(){
    NOP();
}
void CPU::_NOP(uint16_t){
    NOP();
}
void CPU::_SLO(uint16_t addr){
    ASL(addr);
    ORA(addr);
}
void CPU::_RLA(uint16_t addr){
    ROL(addr);
    AND(addr);
}
void CPU::_SRE(uint16_t addr){
    LSR(addr);
    EOR(addr);
}
void CPU::_RRA(uint16_t addr){
    ROR(addr);
    ADC(addr);
}
void CPU::_SAX(uint16_t addr){
    write(addr,A&X);
}
void CPU::_LAX(uint16_t addr){
    LDA(addr);
    LDX(addr);
}
void CPU::_DCP(uint16_t addr){
    DEC(addr);
    CMP(addr);
}
void CPU::_ISC(uint16_t addr){
    INC(addr);
    SBC(addr);
}
void CPU::_ANC(uint16_t addr){
    AND(addr);
    updateC(A<<1);//ASL or ROL
}
void CPU::_ALR(uint16_t addr){
    AND(addr);
    LSR();
}
void CPU::_ARR(uint16_t addr){
    //AND
    uint8_t operand=read(addr);
    A&=operand;
    //ADC V_flag is set before ROR.
    updateV(A,A+operand);
    //ROR(bit 0 does NOT go into carry,but bit 7 is exchanged with the carry.)
    A=updateC((A&0x08)<<1|C<<7|A>>1);
    updateNZ(A);
}
void CPU::_XAA(uint16_t addr){
    TXA();
    AND(addr);
}
void CPU::_LAXi(uint16_t addr){
    LDA(addr);
    TAX();
}
void CPU::_AXS(uint16_t addr){
    //CMP and DEX
    X=updateC(A&X-read(addr));
    updateNZ(X);
}
void CPU::_SBC(uint16_t addr){
    SBC(addr);
    NOP();
}
void CPU::_AHX(uint16_t addr){
    write(addr,A&((addr>>8)+1)&X);
}
void CPU::_SHY(uint16_t addr){
    write(addr,((addr>>8)+1)&Y);
}
void CPU::_SHX(uint16_t addr){
    write(addr,((addr>>8)+1)&X);
}
void CPU::_TAS(uint16_t addr){
    SP=A&X;
    write(addr,SP&((addr>>8)+1));
}
void CPU::_LAS(uint16_t addr){
    A=X=SP=read(addr)&SP;
    updateNZ(A);
}

void CPU::dma_start(uint8_t addr){
    cyc.dma();
    dma_base_addr=static_cast<uint16_t>(addr<<8);
    dma_running=true;
}
void CPU::dma_exec(){
    cyc.dma();
    static uint8_t data;
    if(dma_parity^=1){
        data=read(dma_base_addr|dma_index);
    }else{
        ppu.write(4,data);
        if(++dma_index==0)
            dma_running=false;
    }
}
constexpr const char* CPU::mnemonic[0x100];