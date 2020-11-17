#pragma once

#include <cstdint>
#include "CpuState.h"

class Bus;

class Cpu
{
public:
    Cpu(Bus& bus);

    void Reset();

    void SetIrq(bool irq);

    void SignalNmi();

    void RunInstruction();

    void CaptureState(CpuState* state) const;
    void RestoreState(const CpuState& state);

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
    void LoadZeroPage();
    void Store();
    void StoreZeroPage();
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
    void SaxZeroPage();
    void Sec();
    void Sed();
    void Sei();
    void Sbc();
    void Shx();
    void Shy();
    void Slo();
    void Sre();
    void Sta();
    void StaZeroPage();
    void Stp();
    void Stx();
    void StxZeroPage();
    void Sty();
    void StyZeroPage();
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
    uint8_t ReadDataZeroPage(uint16_t address);
    uint8_t Pop();

    void Write(uint16_t address, uint8_t value);
    void WriteZeroPage(uint16_t address, uint8_t value);
    void Push(uint8_t value);

    Bus& bus_;

    uint16_t address_{};

    uint8_t inValue_{};
    uint8_t outValue_{};

    CpuState state_;
};