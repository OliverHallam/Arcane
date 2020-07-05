#pragma once

#include <cstdint>

class Bus;

class Cpu
{
public:
    Cpu(Bus& bus);

    void Reset();

    void SetIrq(bool irq);

    void SignalNmi();

    void RunInstruction();

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
    void Ahx();
    void Anc();
    void And();
    void Asl();
    void Axa();
    void Axs();
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
    void Dcp();
    void Dec();
    void Dex();
    void Dey();
    void Eor();
    void Inc();
    void Inx();
    void Iny();
    void Isc();
    void Jmp();
    void Las();
    void Lax();
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
    void Rla();
    void Rol();
    void Ror();
    void Rra();
    void Rti();
    void Rts();
    void Sax();
    void Sec();
    void Sed();
    void Sei();
    void Sbc();
    void Shx();
    void Shy();
    void Slo();
    void Sre();
    void Sta();
    void Stp();
    void Stx();
    void Sty();
    void Tas();
    void Tax();
    void Tay();
    void Tsx();
    void Txa();
    void Txs();
    void Tya();
    void Xaa();

    void Jump();
    void SetFlags(uint8_t value);
    void SetCompareFlags(uint16_t result);

    uint8_t ReadProgramByte();
    uint8_t ReadData(uint16_t address);
    uint8_t Pop();

    void Write(uint16_t address, uint8_t value);
    void Push(uint8_t value);

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

    uint8_t inValue_{};
    uint8_t outValue_{};

    uint16_t interruptVector_{};
    bool irq_{};
    bool skipInterrupt_{};
};