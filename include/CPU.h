#pragma once
class Cart;
class PPU;
class RAM;
class Input;

class CPU{
public:
    CPU(Cart*,PPU&);//hard reset(power on)
    ~CPU();
    void run();
private:
    uint8_t read(uint16_t addr);
    void write(uint16_t addr,uint8_t data); 
    void exec();
    void NMI();
    void IRQ();
    Cart* cart;
    PPU& ppu;
    RAM* ram;
    Input* input;
    //Register
    uint8_t A;
    uint8_t X;
    uint8_t Y;
    uint16_t PC;
    uint8_t SP;
    //PSR(processor status register)
    bool N,//Negative flag
         V,//oVerflow flag
         D,//Decimal flag
         I,//Interrupt flag
         Z,//Zero flag
         C;//Carry flag
    enum{
        R=0x20,//Unused.Always 1.
        B=0x10//Break flag dosen't exist.
    };
    //Detect page boundary crossed
    class Cycle{
        bool penalty;
        int cycle=9;
        static constexpr int cycle_table[0x100]={7,6,0,8,3,3,5,5,3,2,2,2,4,4,6,6,2,5,0,8,4,4,6,6,2,4,2,7,4,4,7,7,6,6,0,8,3,3,5,5,4,2,2,2,4,4,6,6,2,5,0,8,4,4,6,6,2,4,2,7,4,4,7,7,6,6,0,8,3,3,5,5,3,2,2,2,3,4,6,6,2,5,0,8,4,4,6,6,2,4,2,7,4,4,7,7,6,6,0,8,3,3,5,5,4,2,2,2,5,4,6,6,2,5,0,8,4,4,6,6,2,4,2,7,4,4,7,7,2,6,2,6,3,3,3,3,2,2,2,2,4,4,4,4,2,6,0,6,4,4,4,4,2,5,2,5,5,5,5,5,2,6,2,6,3,3,3,3,2,2,2,2,4,4,4,4,2,5,0,5,4,4,4,4,2,4,2,4,4,4,4,4,2,6,2,8,3,3,5,5,2,2,2,2,4,4,6,6,2,5,0,8,4,4,6,6,2,4,2,7,4,4,7,7,2,6,2,8,3,3,5,5,2,2,2,2,4,4,6,6,2,5,0,8,4,4,6,6,2,4,2,7,4,4,7,7};
        static constexpr bool penalty_table[0x100]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,1,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,1,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,1,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,1,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,1,0,0,0,0,0,1,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,1,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,1,0,0,1,1,0,0};
    public:
        bool shouldExec();
        void init(uint8_t opcode);
        void nmi();
        void dma();
        void branch();
        uint16_t add(uint8_t,uint8_t);
        uint16_t relative(uint16_t pc,int8_t offset);
    }cyc;
    //Stack Operations
    void push(uint8_t);
    uint8_t pop();
    //Addressing modes(_x represents for indexed X)
    //       implied();
    //       accumulator()
    uint16_t immediate();
    uint16_t zeropage();
    uint16_t absolute();
    uint16_t zeropage_x();
    uint16_t zeropage_y();
    uint16_t absolute_x();
    uint16_t absolute_y();
    uint16_t zeropage_x_indirect();
    uint16_t zeropage_indirect_y();
    uint16_t absolute_indirect();
    uint16_t relative();
    //Opecodes
    void LDA(uint16_t addr);//LoaD Accumulator
    void LDX(uint16_t addr);//LoaD X register
    void LDY(uint16_t addr);//LoaD Y resigter
    void STA(uint16_t addr);//STore Accumulator
    void STX(uint16_t addr);//STore X register
    void STY(uint16_t addr);//Store Y register
    void ADC(uint16_t addr);//ADd with Caryy
    void SBC(uint16_t addr);//SuBtract with Carry
    void CPX(uint16_t addr);//ComPare X register
    void CPY(uint16_t addr);//ComPare Y register
    void CMP(uint16_t addr);//CoMPare accumulator
    void AND(uint16_t addr);//bitwise AND with accumulator
    void EOR(uint16_t addr);//bitwise Exclusive OR
    void ORA(uint16_t addr);//bitwise OR with Accumulator
    void BIT(uint16_t addr);//test BITs
    void ASL();             //Arithmetic Shift Left
    void ASL(uint16_t addr);
    void LSR();             //Logical Shift Right
    void LSR(uint16_t addr);
    void ROL();             //ROtate Left
    void ROL(uint16_t addr);
    void ROR();             //ROtate Right
    void ROR(uint16_t addr);
    void INC(uint16_t addr);//INCrement memory
    void DEC(uint16_t addr);//DECrement memory
    void NOP();             //No OPeration
    void BRK();             //BReaK
    void RTI();             //ReTurn from Interrupt
    void RTS();             //ReTurn from Subroutine
    void JSR(uint16_t addr);//Jump to SubRoutine
    void JMP(uint16_t addr);//JuMP
    //Branch Instructions
    void branch(bool);
    void BPL();//Branch on PLus
    void BMI();//Branch on MInus
    void BVC();//Branch on oVerflow Clear
    void BVS();//Branch on oVerflow Set
    void BCC();//Branch on Carry Clear
    void BCS();//Branch on Carry Set
    void BNE();//Branch on Not Equal
    void BEQ();//Branch on EQual
    //Flag Instructions
    void CLC();//CLear Caryy
    void SEC();//SEt Carry
    void CLI();//CLear Interrupt
    void SEI();//SEt Interrupt
    void CLV();//CLear oVerflow
    void CLD();//CLear Decimal
    void SED();//SEt Decimal
    //Register Instructions
    void TAX();//Transfer A to X
    void TXA();//Transfer X to A
    void DEX();//DEcrement X
    void INX();//INcrement X
    void TAY();//Transfer A to Y
    void TYA();//Transfer Y to A
    void DEY();//DEcrement Y
    void INY();//INcrement Y
    //Stack Instructions
    void TXS();//Transfer X to Stack ptr
    void TSX();//Transfer Stack ptr to X
    void PHA();//PusH Accumulator
    void PLA();//PuLl Accumulator
    void PHP();//PusH Processor status
    void PLP();//PuLl Processor status

    //Set and unset flags
    void    updateNZ(uint8_t operand);
    void    updateV(int opreand, int result);
    uint8_t updateC(int operand);
    //Utility
    void    setPSR(uint8_t data);
    uint8_t PSR_to_uint8_t();
    //Unofficial opcodes
    void _NOP();
    void _NOP(uint16_t addr);   //Only addressing.(+page boundary penalty)
    void _SLO(uint16_t addr);   //ASL+ORA  (a.k.a.)
    void _RLA(uint16_t addr);   //ROL+AND
    void _SRE(uint16_t addr);   //LSR+EOR
    void _RRA(uint16_t addr);   //ROR+ADC
    void _SAX(uint16_t addr);   //Store A and X registers---(*1)
    void _LAX(uint16_t addr);   //LDA+LDX
    void _DCP(uint16_t addr);   //DEC+CMP
    void _ISC(uint16_t addr);   //INC+SBC
    void _ANC(uint16_t addr);   //AND(imm)+(ASL or ROL)
    void _ALR(uint16_t addr);   //AND(imm)+LSR
    void _ARR(uint16_t addr);   //AND(imm)+(ROR + ADC)
    void _XAA(uint16_t addr);   //TXA+AND(imm)----(*2)
    void _LAXi(uint16_t addr);  //LDA(imm)+TAX----(*2)
    void _AXS(uint16_t addr);   //A&X minus #{imm} into X---(*3)
    void _SBC(uint16_t addr);   //SBC(imm)+NOP
    void _AHX(uint16_t addr);   //store A&H(*4)&X---(*5)
    void _SHY(uint16_t addr);   //Store H&Y---------(*5)
    void _SHX(uint16_t addr);   //Store H&X---------(*5)
    void _TAS(uint16_t addr);   //(STA+TXS)---------(*5)
    void _LAS(uint16_t addr);   //(LDA+TSX)
    // 1: A and X put onto the bus at the same time.
    // 2: Highly unstable (results are not predictable on some machines).Do NOT USE!!
    // 3: AXS performs CMP and DEX.
    // 4: H=High byte of the target address +1.
    // 5: Unstable in certain matters
    //----A.k.a.----
    // SLO=ASO
    // SRE=LSE
    // SAX=AAX=AXS
    // ISC=ISB
    // ANC=AAC
    // ALR=ASR
    // AHX=SHA
    // SHY=A11
    // SHX=A11
    // TAS=XAS=SHS
    // LAS=LAR
    // KIL=JAM,HLT,STP

    //DMA 
    bool dma_running=false;
    void dma_start(uint8_t base_addr); 
    void dma_exec();
    uint16_t dma_base_addr;
    uint8_t dma_index=0;
    bool dma_parity=false;

    static constexpr const char* mnemonic[0x100]={"BRK","ORA","KIL","SLO","NOP","ORA","ASL","SLO","PHP","ORA","ASL","ANC","NOP","ORA","ASL","SLO","BPL","ORA","KIL","SLO","NOP","ORA","ASL","SLO","CLC","ORA","NOP","SLO","NOP","ORA","ASL","SLO","JSR","AND","KIL","RLA","BIT","AND","ROL","RLA","PLP","AND","ROL","ANC","BIT","AND","ROL","RLA","BMI","AND","KIL","RLA","NOP","AND","ROL","RLA","SEC","AND","NOP","RLA","NOP","AND","ROL","RLA","RTI","EOR","KIL","SRE","NOP","EOR","LSR","SRE","PHA","EOR","LSR","ALR","JMP","EOR","LSR","SRE","BVC","EOR","KIL","SRE","NOP","EOR","LSR","SRE","CLI","EOR","NOP","SRE","NOP","EOR","LSR","SRE","RTS","ADC","KIL","RRA","NOP","ADC","ROR","RRA","PLA","ADC","ROR","ARR","JMP","ADC","ROR","RRA","BVS","ADC","KIL","RRA","NOP","ADC","ROR","RRA","SEI","ADC","NOP","RRA","NOP","ADC","ROR","RRA","NOP","STA","NOP","SAX","STY","STA","STX","SAX","DEY","NOP","TXA","XAA","STY","STA","STX","SAX","BCC","STA","KIL","AHX","STY","STA","STX","SAX","TYA","STA","TXS","TAS","SHY","STA","SHX","AHX","LDY","LDA","LDX","LAX","LDY","LDA","LDX","LAX","TAY","LDA","TAX","LAX","LDY","LDA","LDX","LAX","BCS","LDA","KIL","LAX","LDY","LDA","LDX","LAX","CLV","LDA","TSX","LAS","LDY","LDA","LDX","LAX","CPY","CMP","NOP","DCP","CPY","CMP","DEC","DCP","INY","CMP","DEX","AXS","CPY","CMP","DEC","DCP","BNE","CMP","KIL","DCP","NOP","CMP","DEC","DCP","CLD","CMP","NOP","DCP","NOP","CMP","DEC","DCP","CPX","SBC","NOP","ISC","CPX","SBC","INC","ISC","INX","SBC","NOP","SBC","CPX","SBC","INC","ISC","BEQ","SBC","KIL","ISC","NOP","SBC","INC","ISC","SED","SBC","NOP","ISC","NOP","SBC","INC","ISC"};
};