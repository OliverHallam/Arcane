#pragma once

#include <cstdint>

class Bus;

class Cpu
{
public:
    Cpu(Bus& bus);

    void Reset();
    void SignalNmi();

    void RunInstruction();

    void RunDma(uint8_t page);

private:
    void Interrupt();
    void Jsr();

    // addressing modes
    void Implicit();
    void Immediate();
    void Absolute();
    void AbsoluteXRead();
    void AbsoluteXWrite();
    void AbsoluteYRead();
    void AbsoluteYWrite();
    void ZeroPage();
    void ZeroPageX();
    void ZeroPageY();
    void Relative();
    void AbsoluteIndirect();
    void IndexIndirect();
    void IndirectIndexRead();
    void IndirectIndexWrite();

    // load/store "steps" for in-memory operations
    void Load();
    void Store();
    void LoadA();
    void StoreA();

    // instructions
    void Adc();
    void And();
    void Asl();
    void Bcc();
    void Bcs();
    void Beq();
    void Bit();
    void Bmi();
    void Bne();
    void Bpl();
    void Brk();
    void Bvc();
    void Bvs();
    void Clc();
    void Cld();
    void Cli();
    void Clv();
    void Cmp();
    void Cpx();
    void Cpy();
    void Dec();
    void Dex();
    void Dey();
    void Eor();
    void Inc();
    void Inx();
    void Iny();
    void Jmp();
    void Lda();
    void Ldx();
    void Ldy();
    void Lsr();
    void Ora();
    void Nop();
    void Pha();
    void Php();
    void Pla();
    void Plp();
    void Rol();
    void Ror();
    void Rti();
    void Rts();
    void Sec();
    void Sed();
    void Sei();
    void Sbc();
    void Sta();
    void Stx();
    void Sty();
    void Tax();
    void Tay();
    void Tsx();
    void Txa();
    void Txs();
    void Tya();

    void Jump();
    void SetFlags(uint8_t value);
    void SetCompareFlags(uint8_t reg);

    uint8_t ReadProgramByte();
    uint8_t ReadData(uint16_t address);
    uint8_t Pop();

    void Write(uint16_t address, uint8_t value);
    void Push(uint8_t value);

    void Tick();

    Bus& bus_;

    // Registers
    uint8_t A_{};
    uint8_t X_{};
    uint8_t Y_{};
    uint16_t PC_{};
    uint8_t S_{};

    // the flags register as seperate bytes
    bool C_{}, Z_{}, I_{}, D_{}, B_{}, V_{}, N_{};
    uint8_t P();
    void P(uint8_t value);

    uint16_t address_{};
    uint8_t value_{};

    uint16_t interruptVector_{};

    uint32_t cycleCount_;
};