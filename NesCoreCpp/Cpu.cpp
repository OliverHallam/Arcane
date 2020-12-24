#include "Cpu.h"

#include "Bus.h"

Cpu::Cpu(Bus& bus) :
    bus_{ bus }
{
}

void Cpu::Reset()
{
    // TODO: strictly, this probably shouldn't unhalt the CPU
    state_.InterruptVector = 0xfffc;
}

void Cpu::SetIrq(bool irq)
{
    state_.Irq = irq;
    if (irq)
    {
        if (!state_.I && state_.InterruptVector == 0)
            state_.InterruptVector = 0xfffe;
    }
    else
    {
        if (state_.InterruptVector == 0xfffe)
            state_.InterruptVector = 0;
    }
}

void Cpu::Nmi()
{
    if (state_.InterruptVector != 1)
        state_.InterruptVector = 0xfffa;
}

void Cpu::RunInstruction()
{
    if (state_.InterruptVector != 0)
    {
        if (state_.SkipInterrupt)
        {
            state_.SkipInterrupt = false;
        }
        else
        {
            if (state_.InterruptVector != 0xfffe)
            {
                bus_.CpuDummyRead(state_.PC);

                if (state_.InterruptVector == 1)
                    return;

                bus_.CpuDummyRead(state_.PC);
            }

            Interrupt();
            state_.InterruptVector = 0;
            return;
        }
    }

    auto opCode = ReadProgramByte();

    switch (opCode)
    {
    case 0x00:
        Implicit();
        Brk();
        return;

    case 0x01:
        IndexIndirect();
        LoadZeroPage();
        Ora();
        return;

    case 0x02:
        Stp();
        return;

    case 0x03:
        IndexIndirect();
        Load();
        Slo();
        Store();
        return;

    case 0x04:
        ZeroPage();
        bus_.CpuDummyRead(address_);
        Nop();
        return;

    case 0x05:
        ZeroPage();
        LoadZeroPage();
        Ora();
        return;

    case 0x06:
        ZeroPage();
        LoadZeroPage();
        Asl();
        StoreZeroPage();
        return;

    case 0x07:
        ZeroPage();
        LoadZeroPage();
        Slo();
        StoreZeroPage();
        return;

    case 0x08:
        Implicit();
        Php();
        return;

    case 0x09:
        Immediate();
        Ora();
        return;

    case 0x0a:
        Implicit();
        LoadA();
        Asl();
        StoreA();
        return;

    case 0x0b:
        Immediate();
        Anc();
        return;

    case 0x0c:
        Absolute();
        bus_.CpuDummyRead(address_);
        Nop();
        return;

    case 0x0d:
        Absolute();
        Load();
        Ora();
        return;

    case 0x0e:
        Absolute();
        Load();
        Asl();
        Store();
        return;

    case 0x0f:
        Absolute();
        Load();
        Slo();
        Store();
        return;

    case 0x10:
        Relative();
        Bpl();
        return;

    case 0x11:
        IndirectIndexRead();
        Load();
        Ora();
        return;

    case 0x12:
        Stp();
        return;

    case 0x13:
        IndirectIndexRead();
        Load();
        Slo();
        Store();
        return;

    case 0x14:
        ZeroPageX();
        bus_.CpuDummyRead(address_);
        Nop();
        return;

    case 0x15:
        ZeroPageX();
        LoadZeroPage();
        Ora();
        return;

    case 0x16:
        ZeroPageX();
        LoadZeroPage();
        Asl();
        StoreZeroPage();
        return;

    case 0x17:
        ZeroPageX();
        LoadZeroPage();
        Slo();
        StoreZeroPage();
        return;

    case 0x18:
        Implicit();
        Clc();
        return;

    case 0x19:
        AbsoluteYRead();
        Load();
        Ora();
        return;

    case 0x1a:
        Implicit();
        Nop();
        return;

    case 0x1b:
        AbsoluteYWrite();
        Load();
        Slo();
        Store();
        return;

    case 0x1c:
        AbsoluteXRead();
        bus_.CpuDummyRead(address_);
        Nop();
        return;

    case 0x1d:
        AbsoluteXRead();
        Load();
        Ora();
        return;

    case 0x1e:
        AbsoluteXWrite();
        Load();
        Asl();
        Store();
        return;

    case 0x1f:
        AbsoluteXWrite();
        Load();
        Slo();
        Store();
        return;

    case 0x20:
        // timings are a little different on this one, so the decoding happens in the instruction
        Jsr();
        return;

    case 0x21:
        IndexIndirect();
        Load();
        And();
        return;

    case 0x22:
        Immediate();
        Stp();
        return;

    case 0x23:
        IndexIndirect();
        Load();
        Rla();
        Store();
        return;

    case 0x24:
        ZeroPage();
        LoadZeroPage();
        Bit();
        return;

    case 0x25:
        ZeroPage();
        LoadZeroPage();
        And();
        return;

    case 0x26:
        ZeroPage();
        LoadZeroPage();
        Rol();
        StoreZeroPage();
        return;

    case 0x27:
        ZeroPage();
        LoadZeroPage();
        Rla();
        StoreZeroPage();
        return;

    case 0x28:
        Implicit();
        Plp();
        return;

    case 0x29:
        Immediate();
        And();
        return;

    case 0x2a:
        Implicit();
        LoadA();
        Rol();
        StoreA();
        return;

    case 0x2b:
        Immediate();
        Anc();
        return;

    case 0x2c:
        Absolute();
        Load();
        Bit();
        return;

    case 0x2d:
        Absolute();
        Load();
        And();
        return;

    case 0x2e:
        Absolute();
        Load();
        Rol();
        Store();
        return;

    case 0x2f:
        Absolute();
        Load();
        Rla();
        Store();
        return;

    case 0x30:
        Relative();
        Bmi();
        return;

    case 0x31:
        IndirectIndexRead();
        Load();
        And();
        return;

    case 0x32:
        Implicit();
        Stp();
        return;

    case 0x33:
        IndirectIndexWrite();
        Load();
        Rla();
        Store();
        return;

    case 0x34:
        ZeroPageX();
        bus_.CpuDummyRead(address_);
        Nop();
        return;

    case 0x35:
        ZeroPageX();
        LoadZeroPage();
        And();
        return;

    case 0x36:
        ZeroPageX();
        LoadZeroPage();
        Rol();
        StoreZeroPage();
        return;

    case 0x37:
        ZeroPageX();
        LoadZeroPage();
        Rla();
        StoreZeroPage();
        return;

    case 0x38:
        Implicit();
        Sec();
        return;

    case 0x39:
        AbsoluteYRead();
        Load();
        And();
        return;

    case 0x3a:
        Implicit();
        Nop();
        return;

    case 0x3b:
        AbsoluteYWrite();
        Load();
        Rla();
        Store();
        return;

    case 0x3c:
        AbsoluteXRead();
        bus_.CpuDummyRead(address_);
        Nop();
        return;

    case 0x3d:
        AbsoluteXRead();
        Load();
        And();
        return;

    case 0x3e:
        AbsoluteXWrite();
        Load();
        Rol();
        Store();
        return;

    case 0x3f:
        AbsoluteXWrite();
        Load();
        Rla();
        Store();
        return;

    case 0x40:
        Implicit();
        Rti();
        return;

    case 0x41:
        IndexIndirect();
        Load();
        Eor();
        return;

    case 0x42:
        Implicit();
        Stp();
        return;

    case 0x43:
        IndexIndirect();
        Load();
        Sre();
        Store();
        return;

    case 0x44:
        ZeroPage();
        bus_.CpuDummyRead(address_);
        Nop();
        return;

    case 0x45:
        ZeroPage();
        LoadZeroPage();
        Eor();
        return;

    case 0x46:
        ZeroPage();
        LoadZeroPage();
        Lsr();
        StoreZeroPage();
        return;

    case 0x47:
        ZeroPage();
        LoadZeroPage();
        Sre();
        StoreZeroPage();
        return;

    case 0x48:
        Implicit();
        Pha();
        return;

    case 0x49:
        Immediate();
        Eor();
        return;

    case 0x4a:
        Implicit();
        LoadA();
        Lsr();
        StoreA();
        return;

    case 0x4b:
        // ALR
        Immediate();
        And();
        LoadA();
        Lsr();
        StoreA();
        return;

    case 0x4c:
        Absolute();
        Jmp();
        return;

    case 0x4d:
        Absolute();
        Load();
        Eor();
        return;

    case 0x4e:
        Absolute();
        Load();
        Lsr();
        Store();
        return;

    case 0x4f:
        Absolute();
        Load();
        Sre();
        Store();
        return;

    case 0x50:
        Relative();
        Bvc();
        return;

    case 0x51:
        IndirectIndexRead();
        Load();
        Eor();
        return;

    case 0x52:
        Implicit();
        Stp();
        return;

    case 0x53:
        IndirectIndexWrite();
        Load();
        Sre();
        Store();
        return;

    case 0x54:
        ZeroPageX();
        bus_.CpuDummyRead(address_);
        Nop();
        return;

    case 0x55:
        ZeroPageX();
        LoadZeroPage();
        Eor();
        return;

    case 0x56:
        ZeroPageX();
        LoadZeroPage();
        Lsr();
        StoreZeroPage();
        return;

    case 0x57:
        ZeroPageX();
        LoadZeroPage();
        Sre();
        StoreZeroPage();
        return;

    case 0x58:
        Implicit();
        Cli();
        return;

    case 0x59:
        AbsoluteYRead();
        Load();
        Eor();
        return;

    case 0x5a:
        Implicit();
        Nop();
        return;

    case 0x5b:
        AbsoluteYWrite();
        Load();
        Sre();
        Store();
        return;

    case 0x5c:
        AbsoluteXRead();
        bus_.CpuDummyRead(address_);
        Nop();
        return;

    case 0x5d:
        AbsoluteXRead();
        Load();
        Eor();
        return;

    case 0x5e:
        AbsoluteXWrite();
        Load();
        Lsr();
        Store();
        return;

    case 0x5f:
        AbsoluteXWrite();
        Load();
        Sre();
        Store();
        return;

    case 0x60:
        Implicit();
        Rts();
        return;

    case 0x61:
        IndexIndirect();
        Load();
        Adc();
        return;

    case 0x62:
        Implicit();
        Stp();
        return;

    case 0x63:
        IndexIndirect();
        Load();
        Rra();
        Store();
        return;

    case 0x64:
        ZeroPage();
        bus_.CpuDummyRead(address_);
        Nop();
        return;

    case 0x65:
        ZeroPage();
        LoadZeroPage();
        Adc();
        return;

    case 0x66:
        ZeroPage();
        LoadZeroPage();
        Ror();
        StoreZeroPage();
        return;

    case 0x67:
        ZeroPage();
        LoadZeroPage();
        Rra();
        StoreZeroPage();
        return;

    case 0x68:
        Implicit();
        Pla();
        return;

    case 0x69:
        Immediate();
        Adc();
        return;

    case 0x6a: 
        Implicit();
        LoadA();
        Ror();
        StoreA();
        return;

    case 0x6b:
        Immediate();
        And();
        LoadA();
        Ror();
        StoreA();
        return;

    case 0x6c:
        AbsoluteIndirect();
        Jmp();
        return;

    case 0x6d:
        Absolute();
        Load();
        Adc();
        return;

    case 0x6e:
        Absolute();
        Load();
        Ror();
        Store();
        return;

    case 0x6f:
        Absolute();
        Load();
        Rra();
        Store();
        return;

    case 0x70:
        Relative();
        Bvs();
        return;

    case 0x71:
        IndirectIndexRead();
        Load();
        Adc();
        return;

    case 0x72:
        Implicit();
        Stp();
        return;

    case 0x73:
        IndirectIndexWrite();
        Load();
        Rra();
        Store();
        return;

    case 0x74:
        ZeroPageX();
        bus_.CpuDummyRead(address_);
        Nop();
        return;

    case 0x75:
        ZeroPageX();
        LoadZeroPage();
        Adc();
        return;

    case 0x76:
        ZeroPageX();
        LoadZeroPage();
        Ror();
        StoreZeroPage();
        return;

    case 0x77:
        ZeroPageX();
        LoadZeroPage();
        Rra();
        StoreZeroPage();
        return;

    case 0x78:
        Implicit();
        Sei();
        return;

    case 0x79:
        AbsoluteYRead();
        Load();
        Adc();
        return;

    case 0x7a:
        Implicit();
        Nop();
        return;

    case 0x7b:
        AbsoluteYWrite();
        Load();
        Rra();
        Store();
        return;

    case 0x7c:
        AbsoluteXRead();
        bus_.CpuDummyRead(address_);
        Nop();
        return;

    case 0x7d:
        AbsoluteXRead();
        Load();
        Adc();
        return;

    case 0x7e:
        AbsoluteXWrite();
        Load();
        Ror();
        Store();
        return;

    case 0x7f:
        AbsoluteXWrite();
        Load();
        Rra();
        Store();
        return;

    case 0x80:
        Immediate();
        Nop();
        return;

    case 0x81:
        IndexIndirect();
        Sta();
        return;

    case 0x82:
        Immediate();
        Nop();
        return;

    case 0x83:
        IndexIndirect();
        Sax();
        return;

    case 0x84:
        ZeroPage();
        StyZeroPage();
        return;

    case 0x85:
        ZeroPage();
        StaZeroPage();
        return;

    case 0x86:
        ZeroPage();
        StxZeroPage();
        return;

    case 0x87:
        ZeroPage();
        SaxZeroPage();
        return;

    case 0x88:
        Implicit();
        Dey();
        return;

    case 0x89:
        Immediate();
        Nop();
        return;

    case 0x8a:
        Implicit();
        Txa();
        return;

    case 0x8b:
        Immediate();
        Xaa();
        return;

    case 0x8c:
        Absolute();
        Sty();
        return;

    case 0x8d:
        Absolute();
        Sta();
        return;

    case 0x8e:
        Absolute();
        Stx();
        return;

    case 0x8f:
        Absolute();
        Sax();
        return;

    case 0x90:
        Relative();
        Bcc();
        return;

    case 0x91:
        IndirectIndexWrite();
        Sta();
        return;

    case 0x92:
        Implicit();
        Stp();
        return;

    case 0x93:
        IndirectIndexWrite();
        Load();
        Axa();
        Store();
        return;

    case 0x94:
        ZeroPageX();
        Sty();
        return;

    case 0x95:
        ZeroPageX();
        Sta();
        return;

    case 0x96:
        ZeroPageY();
        Stx();
        return;

    case 0x97:
        ZeroPageY();
        Sax();
        return;

    case 0x98:
        Implicit();
        Tya();
        return;

    case 0x99:
        AbsoluteYWrite();
        bus_.CpuDummyRead(address_);
        Sta();
        return;

    case 0x9a:
        Implicit();
        Txs();
        return;

    case 0x9b:
        AbsoluteYWrite();
        Load();
        Tas();
        Store();
        return;

    case 0x9c:
        AbsoluteXWrite();
        Load();
        Shy();
        Store();
        return;

    case 0x9d:
        AbsoluteXWrite();
        Sta();
        return;

    case 0x9e:
        AbsoluteYWrite();
        Load();
        Shx();
        Store();
        return;

    case 0x9f:
        AbsoluteYWrite();
        Load();
        Ahx();
        Store();
        return;

    case 0xa0:
        Immediate();
        Ldy();
        return;

    case 0xa1:
        IndexIndirect();
        Load();
        Lda();
        return;

    case 0xa2:
        Immediate();
        Ldx();
        return;

    case 0xa3:
        IndexIndirect();
        Load();
        Lax();
        return;

    case 0xa4:
        ZeroPage();
        LoadZeroPage();
        Ldy();
        return;

    case 0xa5:
        ZeroPage();
        LoadZeroPage();
        Lda();
        return;

    case 0xa6:
        ZeroPage();
        LoadZeroPage();
        Ldx();
        return;

    case 0xa7:
        ZeroPage();
        LoadZeroPage();
        Lax();
        return;

    case 0xa8:
        Implicit();
        Tay();
        return;

    case 0xa9:
        Immediate();
        Lda();
        return;

    case 0xaa:
        Implicit();
        Tax();
        return;

    case 0xab:
        Immediate();
        Lax();
        return;

    case 0xac:
        Absolute();
        Load();
        Ldy();
        return;

    case 0xad:
        Absolute();
        Load();
        Lda();
        return;

    case 0xae:
        Absolute();
        Load();
        Ldx();
        return;

    case 0xaf:
        Absolute();
        Load();
        Lax();
        return;

    case 0xb0:
        Relative();
        Bcs();
        return;

    case 0xb1:
        IndirectIndexRead();
        Load();
        Lda();
        return;

    case 0xb2:
        Implicit();
        Stp();
        return;

    case 0xb3:
        IndirectIndexRead();
        Load();
        Lax();
        return;

    case 0xb4:
        ZeroPageX();
        LoadZeroPage();
        Ldy();
        return;

    case 0xb5:
        ZeroPageX();
        LoadZeroPage();
        Lda();
        return;

    case 0xb6:
        ZeroPageY();
        LoadZeroPage();
        Ldx();
        return;

    case 0xb7:
        ZeroPageY();
        LoadZeroPage();
        Lax();
        return;

    case 0xb8:
        Implicit();
        Clv();
        return;

    case 0xb9:
        AbsoluteYRead();
        Load();
        Lda();
        return;

    case 0xba:
        Implicit();
        Tsx();
        return;

    case 0xbb:
        AbsoluteYRead();
        Load();
        Las();
        return;

    case 0xbc:
        AbsoluteXRead();
        Load();
        Ldy();
        return;

    case 0xbd:
        AbsoluteXRead();
        Load();
        Lda();
        return;

    case 0xbe:
        AbsoluteYRead();
        Load();
        Ldx();
        return;

    case 0xbf:
        AbsoluteYRead();
        Load();
        Lax();
        return;

    case 0xc0:
        Immediate();
        Cpy();
        return;

    case 0xc1:
        IndexIndirect();
        Load();
        Cmp();
        return;

    case 0xc2:
        Immediate();
        bus_.CpuDummyRead(address_);
        Nop();
        return;

    case 0xc3:
        IndexIndirect();
        Load();
        Dcp();
        Store();
        return;

    case 0xc4:
        ZeroPage();
        LoadZeroPage();
        Cpy();
        return;

    case 0xc5:
        ZeroPage();
        LoadZeroPage();
        Cmp();
        return;

    case 0xc6:
        ZeroPage();
        LoadZeroPage();
        Dec();
        StoreZeroPage();
        return;

    case 0xc7:
        ZeroPage();
        LoadZeroPage();
        Dcp();
        StoreZeroPage();
        return;

    case 0xc8:
        Implicit();
        Iny();
        return;

    case 0xc9:
        Immediate();
        Cmp();
        return;

    case 0xca:
        Implicit();
        Dex();
        return;

    case 0xcb:
        Immediate();
        Axs();
        return;

    case 0xcc:
        Absolute();
        Load();
        Cpy();
        return;

    case 0xcd:
        Absolute();
        Load();
        Cmp();
        return;

    case 0xce:
        Absolute();
        Load();
        Dec();
        Store();
        return;

    case 0xcf:
        Absolute();
        Load();
        Dcp();
        Store();
        return;

    case 0xd0:
        Relative();
        Bne();
        return;

    case 0xd1:
        IndirectIndexRead();
        Load();
        Cmp();
        return;

    case 0xd2:
        Implicit();
        Stp();
        return;

    case 0xd3:
        IndirectIndexWrite();
        Load();
        Dcp();
        Store();
        return;

    case 0xd4:
        ZeroPageX();
        bus_.CpuDummyRead(address_);
        Nop();
        return;

    case 0xd5:
        ZeroPageX();
        LoadZeroPage();
        Cmp();
        return;

    case 0xd6:
        ZeroPageX();
        LoadZeroPage();
        Dec();
        StoreZeroPage();
        return;

    case 0xd7:
        ZeroPageX();
        LoadZeroPage();
        Dcp();
        StoreZeroPage();
        return;

    case 0xd8:
        Implicit();
        Cld();
        return;

    case 0xd9:
        AbsoluteYRead();
        Load();
        Cmp();
        return;

    case 0xda:
        Implicit();
        Nop();
        return;

    case 0xdb:
        AbsoluteYWrite();
        Load();
        Dcp();
        Store();
        return;

    case 0xdc:
        AbsoluteXWrite();
        bus_.CpuDummyRead(address_);
        Nop();
        return;

    case 0xdd:
        AbsoluteXRead();
        Load();
        Cmp();
        return;

    case 0xde:
        AbsoluteXWrite();
        Load();
        Dec();
        Store();
        return;

    case 0xdf:
        AbsoluteXWrite();
        Load();
        Dcp();
        Store();
        return;

    case 0xe0:
        Immediate();
        Cpx();
        return;

    case 0xe1:
        IndexIndirect();
        Load();
        Sbc();
        return;

    case 0xe2:
        Immediate();
        Nop();
        return;

    case 0xe3:
        IndexIndirect();
        Load();
        Isc();
        Store();
        return;

    case 0xe4:
        ZeroPage();
        LoadZeroPage();
        Cpx();
        return;

    case 0xe5:
        ZeroPage();
        LoadZeroPage();
        Sbc();
        return;

    case 0xe6:
        ZeroPage();
        LoadZeroPage();
        Inc();
        StoreZeroPage();
        return;

    case 0xe7:
        ZeroPage();
        LoadZeroPage();
        Isc();
        StoreZeroPage();
        return;

    case 0xe8:
        Implicit();
        Inx();
        return;

    case 0xe9:
        Immediate();
        Sbc();
        return;

    case 0xea:
        Implicit();
        Nop();
        return;

    case 0xeb:
        Immediate();
        Sbc();
        return;

    case 0xec:
        Absolute();
        Load();
        Cpx();
        return;

    case 0xed:
        Absolute();
        Load();
        Sbc();
        return;

    case 0xee:
        Absolute();
        Load();
        Inc();
        Store();
        return;

    case 0xef:
        Absolute();
        Load();
        Isc();
        Store();
        return;

    case 0xf0:
        Relative();
        Beq();
        return;

    case 0xf1:
        IndirectIndexRead();
        Load();
        Sbc();
        return;

    case 0xf2:
        Implicit();
        Stp();
        return;

    case 0xf3:
        IndirectIndexWrite();
        Load();
        Isc();
        Store();
        return;

    case 0xf4:
        ZeroPageX();
        bus_.CpuDummyRead(address_);
        Nop();
        return;

    case 0xf5:
        ZeroPageX();
        LoadZeroPage();
        Sbc();
        return;

    case 0xf6:
        ZeroPageX();
        LoadZeroPage();
        Inc();
        StoreZeroPage();
        return;

    case 0xf7:
        ZeroPageX();
        LoadZeroPage();
        Isc();
        StoreZeroPage();
        return;

    case 0xf8:
        Implicit();
        Sed();
        return;

    case 0xf9:
        AbsoluteYRead();
        Load();
        Sbc();
        return;

    case 0xfa:
        Implicit();
        Nop();
        return;

    case 0xfb:
        AbsoluteYWrite();
        Load();
        Isc();
        Store();
        return;

    case 0xfc:
        AbsoluteXRead();
        bus_.CpuDummyRead(address_);
        Nop();
        return;

    case 0xfd:
        AbsoluteXRead();
        Load();
        Sbc();
        return;

    case 0xfe:
        AbsoluteXWrite();
        Load();
        Inc();
        Store();
        return;

    case 0xff:
        AbsoluteXWrite();
        Load();
        Isc();
        Store();
        return;

    default:
        return;
    }
}

void Cpu::CaptureState(CpuState* state) const
{
    *state = state_;
}

void Cpu::RestoreState(const CpuState& state)
{
    state_ = state;
}

void Cpu::Interrupt()
{
    if (state_.InterruptVector != 0xfffc)
    {
        // don't push on reset
        Push((uint8_t)(state_.PC >> 8));
        Push((uint8_t)(state_.PC & 0xff));

        auto p = (uint8_t)(state_.P() | 0x20);
        p |= (uint8_t)(state_.B ? 0x10 : 0);
        Push(p);
    }
    else
    {
        bus_.CpuDummyRead(state_.PC);
        bus_.CpuDummyRead(state_.PC);
        bus_.CpuDummyRead(state_.PC);
    }

    auto pcLow = ReadData(state_.InterruptVector);
    auto pcHigh = ReadData((uint16_t)(state_.InterruptVector + 1));

    state_.PC = (uint16_t)((pcHigh << 8) | pcLow);

    state_.I = true;
}

void Cpu::Jsr()
{
    auto lowByte = ReadProgramByte();

    bus_.CpuDummyRead(state_.PC);

    Push((uint8_t)(state_.PC >> 8));
    Push((uint8_t)(state_.PC & 0xff));

    auto highByte = ReadProgramByte();
    state_.PC = (uint16_t)(highByte << 8 | lowByte);
}

void Cpu::Implicit()
{
    bus_.CpuDummyRead(state_.PC);
}

void Cpu::Immediate()
{
    inValue_ = ReadProgramByte();
}

void Cpu::Absolute()
{
    auto lowByte = ReadProgramByte();
    auto highByte = ReadProgramByte();

    address_ = (uint16_t)(lowByte | highByte << 8);
}

void Cpu::AbsoluteXRead()
{
    auto lowByte = ReadProgramByte();
    auto highByte = ReadProgramByte();

    address_ = (uint16_t)(lowByte | highByte << 8);
    address_ += state_.X;

    if (highByte != (address_ >> 8))
    {
        bus_.CpuDummyRead(address_ - 0x0100);
    }
}

void Cpu::AbsoluteXWrite()
{
    auto lowByte = ReadProgramByte();
    auto highByte = ReadProgramByte();

    address_ = (uint16_t)(lowByte | highByte << 8);
    address_ += state_.X;

    bus_.CpuDummyRead((highByte << 8) | (address_ & 0xff));
}

void Cpu::AbsoluteYRead()
{
    auto lowByte = ReadProgramByte();
    auto highByte = ReadProgramByte();

    address_ = (uint16_t)(lowByte | highByte << 8);
    address_ += state_.Y;

    if (highByte != (address_ >> 8))
    {
        bus_.CpuDummyRead(state_.PC - 0x0100);
    }
}

void Cpu::AbsoluteYWrite()
{
    auto lowByte = ReadProgramByte();
    auto highByte = ReadProgramByte();

    address_ = (uint16_t)(lowByte | highByte << 8);
    address_ += state_.Y;
}

void Cpu::ZeroPage()
{
    address_ = ReadProgramByte();
}

void Cpu::ZeroPageX()
{
    auto zpAddress = ReadProgramByte();

    bus_.CpuDummyRead(zpAddress);

    address_ = (zpAddress + state_.X) & 0x00ff;
}

void Cpu::ZeroPageY()
{
    auto zpAddress = ReadProgramByte();

    bus_.CpuDummyRead(zpAddress);

    address_ = (zpAddress + state_.Y) & 0x00ff;
}

void Cpu::Relative()
{
    auto relative = (int8_t)ReadProgramByte();
    address_ = (uint16_t)(state_.PC + relative);
}

void Cpu::AbsoluteIndirect()
{
    auto addressLow = ReadProgramByte();
    auto addressHigh = ReadProgramByte();

    auto address = (uint16_t)((addressHigh << 8) | addressLow);

    addressLow++;
    auto nextAddress = (uint16_t)((addressHigh << 8) | addressLow);

    addressLow = ReadData(address);
    addressHigh = ReadData(nextAddress);

    address_ = (uint16_t)((addressHigh << 8) | addressLow);
}

void Cpu::IndexIndirect()
{
    uint8_t indirectAddress = ReadProgramByte();

    bus_.CpuDummyRead(indirectAddress);

    indirectAddress += state_.X;

    auto addressLow = ReadData(indirectAddress);
    auto addressHigh = ReadData((uint8_t)(indirectAddress + 1));

    address_ = (uint16_t)(addressLow | addressHigh << 8);
}

void Cpu::IndirectIndexRead()
{
    auto indirectAddress = ReadProgramByte();

    auto addressLow = ReadData(indirectAddress);
    auto addressHigh = ReadData((uint8_t)(indirectAddress + 1));

    address_ = (uint16_t)(addressLow | addressHigh << 8);
    address_ += state_.Y;

    if ((address_ >> 8) != addressHigh)
    {
        // page crossed
        bus_.CpuDummyRead(address_ - 0x0100);
    }
}

void Cpu::IndirectIndexWrite()
{
    auto indirectAddress = ReadProgramByte();

    auto addressLow = ReadData(indirectAddress);
    auto addressHigh = ReadData((uint8_t)(indirectAddress + 1));

    address_ = (uint16_t)(addressLow | addressHigh << 8);

    bus_.CpuDummyRead(address_);

    address_ += state_.Y;
}

void Cpu::Load()
{
    inValue_ = ReadData(address_);
}

void Cpu::LoadZeroPage()
{
    inValue_ = ReadDataZeroPage(address_);
}

void Cpu::Store()
{
    bus_.CpuWrite2(address_, inValue_, outValue_);
}

void Cpu::StoreZeroPage()
{
    // the first value can't have any side effects
    bus_.TickCpuWrite();
    bus_.CpuWriteZeroPage(address_, outValue_);
}

void Cpu::LoadA()
{
    inValue_ = state_.A;
}

void Cpu::StoreA()
{
    state_.A = outValue_;
}

void Cpu::Adc()
{
    auto result = state_.A + inValue_;

    if (state_.C)
    {
        result++;
    }

    state_.V = (~(state_.A ^ inValue_) & (state_.A ^ result) & 0x80) != 0;

    state_.A = static_cast<uint8_t>(result);

    state_.N = (state_.A & 0x80) != 0;
    state_.Z = state_.A == 0;
    state_.C = (result & 0x100) != 0;
}

void Cpu::Ahx()
{
    outValue_ = state_.A & state_.X & ((address_ >> 1) + 1);
}

void Cpu::Anc()
{
    And();
    state_.C = state_.N;
}

void Cpu::And()
{
    state_.A &= inValue_;
    SetFlags(state_.A);
}

void Cpu::Asl()
{
    state_.C = (inValue_ & 0x80) != 0;

    outValue_ = inValue_ << 1;

    state_.Z = outValue_ == 0;
    state_.N = (outValue_ & 0x80) != 0;
}

void Cpu::Axa()
{
    Write(address_, state_.A & state_.X & ((address_ >> 8) + 1));
}

void Cpu::Axs()
{
    auto result = state_.A & state_.X - inValue_;
    state_.X = static_cast<uint8_t>(result);
    SetCompareFlags(result);
}

void Cpu::Bcc()
{
    if (!state_.C)
        Jump();
}

void Cpu::Bcs()
{
    if (state_.C)
        Jump();
}

void Cpu::Beq()
{
    if (state_.Z)
        Jump();
}

void Cpu::Bit()
{
    state_.N = (inValue_ & 0x80) != 0;
    state_.V = (inValue_ & 0x40) != 0;
    state_.Z = (inValue_ & state_.A) == 0;
}

void Cpu::Bmi()
{
    if (state_.N)
        Jump();
}

void Cpu::Bne()
{
    if (!state_.Z)
        Jump();
}

void Cpu::Bpl()
{
    if (!state_.N)
        Jump();
}

void Cpu::Brk()
{
    state_.B = true;
    state_.PC++;
    state_.InterruptVector = 0xfffe;
}

void Cpu::Bvc()
{
    if (!state_.V)
        Jump();
}

void Cpu::Bvs()
{
    if (state_.V)
        Jump();
}

void Cpu::Clc()
{
    state_.C = false;
}

void Cpu::Cld()
{
    state_.D = false;
}

void Cpu::Cli()
{
    state_.I = false;
    if (state_.Irq && state_.InterruptVector == 0)
    {
        state_.InterruptVector = 0xfffe;
        state_.SkipInterrupt = true;
    }
}

void Cpu::Clv()
{
    state_.V = false;
}

void Cpu::Cmp()
{
    SetCompareFlags(state_.A - inValue_);
}

void Cpu::Cpx()
{
    SetCompareFlags(state_.X - inValue_);
}

void Cpu::Cpy()
{
    SetCompareFlags(state_.Y - inValue_);
}

void Cpu::Dcp()
{
    outValue_ = inValue_ - 1;
    SetCompareFlags(state_.A - outValue_);
}

void Cpu::Dec()
{
    outValue_ = inValue_ - 1;
    SetFlags(outValue_);
}

void Cpu::Dex()
{
    state_.X--;
    SetFlags(state_.X);
}

void Cpu::Dey()
{
    state_.Y--;
    SetFlags(state_.Y);
}

void Cpu::Eor()
{
    state_.A ^= inValue_;
    SetFlags(state_.A);
}

void Cpu::Inc()
{
    outValue_ = inValue_ + 1;
    SetFlags(outValue_);
}

void Cpu::Inx()
{
    state_.X++;
    SetFlags(state_.X);
}

void Cpu::Iny()
{
    state_.Y++;
    SetFlags(state_.Y);
}

void Cpu::Isc()
{
    outValue_ = inValue_ + 1;

    auto result = (uint16_t)state_.A - outValue_;
    if (!state_.C)
    {
        result--;
    }

    state_.V = ((state_.A ^ inValue_) & (state_.A ^ result) & 0x80) != 0;

    state_.A = static_cast<uint8_t>(result);
    SetCompareFlags(result);
}

void Cpu::Jmp()
{
    state_.PC = address_;
}

void Cpu::Las()
{
    auto value = state_.S & inValue_;
    state_.A = value;
    state_.X = value;
    state_.S = value;
    SetFlags(value);
}

void Cpu::Lax()
{
    state_.A = inValue_;
    state_.X = inValue_;
    SetFlags(inValue_);
}

void Cpu::Lda()
{
    state_.A = inValue_;
    SetFlags(state_.A);
}

void Cpu::Ldx()
{
    state_.X = inValue_;
    SetFlags(state_.X);
}

void Cpu::Ldy()
{
    state_.Y = inValue_;
    SetFlags(state_.Y);
}

void Cpu::Lsr()
{
    state_.C = (inValue_ & 0x01) != 0;

    outValue_ = inValue_ >> 1;

    state_.N = false; // top bit is left unset
    state_.Z = outValue_ == 0;
}

void Cpu::Ora()
{
    state_.A |= inValue_;
    SetFlags(state_.A);
}

void Cpu::Nop()
{
}

void Cpu::Pha()
{
    Push(state_.A);
}

void Cpu::Php()
{
    Push(state_.P() | 0x30);
}

void Cpu::Pla()
{
    bus_.CpuDummyRead(state_.PC);

    state_.A = Pop();

    SetFlags(state_.A);
}

void Cpu::Plp()
{
    bus_.CpuDummyRead(state_.PC);

    state_.P(Pop());

    if (!state_.I && state_.Irq && state_.InterruptVector == 0)
    {
        state_.InterruptVector = 0xfffe;
        state_.SkipInterrupt = true;
    }
}

void Cpu::Rla()
{
    auto carry = (inValue_ & 0x80) != 0;

    outValue_ = inValue_ << 1;

    if (state_.C)
        outValue_ |= 0x01;

    state_.C = carry;
    state_.A &= outValue_;
    SetFlags(state_.A);
}

void Cpu::Rol()
{
    auto carry = (inValue_ & 0x80) != 0;

    outValue_ = inValue_ << 1;

    if (state_.C)
        outValue_ |= 0x01;

    state_.Z = outValue_ == 0;
    state_.N = (outValue_ & 0x80) != 0;
    state_.C = carry;
}

void Cpu::Ror()
{
    auto carry = (inValue_ & 1) != 0;

    outValue_ = inValue_ >> 1;

    if (state_.C)
        outValue_ |= 0x80;

    state_.Z = outValue_ == 0;
    state_.N = (outValue_ & 0x80) != 0;
    state_.C = carry;
}

void Cpu::Rra()
{
    auto carry = (inValue_ & 1);

    outValue_ = inValue_ >> 1;

    if (state_.C)
        outValue_ |= 0x80;

    auto result = state_.A + outValue_ + carry;

    state_.V = (~(state_.A ^ outValue_) & (state_.A ^ result) & 0x80) != 0;

    state_.A = static_cast<uint8_t>(result);

    state_.N = (state_.A & 0x80) != 0;
    state_.Z = state_.A == 0;
    state_.C = (result & 0x100) != 0;
}

void Cpu::Rti()
{
    bus_.CpuDummyRead(state_.PC);

    state_.P(Pop());
    auto pcLow = Pop();
    auto pcHigh = Pop();

    state_.PC = (uint16_t)(pcHigh << 8 | pcLow);

    if (!state_.I && state_.Irq && state_.InterruptVector == 0)
    {
        state_.InterruptVector = 0xfffe;
    }
}

void Cpu::Rts()
{
    auto lowByte = Pop();
    auto highByte = Pop();

    bus_.CpuDummyRead(state_.PC);
    bus_.CpuDummyRead(state_.PC);

    state_.PC = (uint16_t)(((highByte << 8) | lowByte) + 1);
}

void Cpu::Sax()
{
    Write(address_, state_.A & state_.X);
}

void Cpu::SaxZeroPage()
{
    WriteZeroPage(address_, state_.A & state_.X);
}

void Cpu::Sbc()
{
    auto result = (uint16_t)state_.A - inValue_;
    if (!state_.C)
    {
        result--;
    }

    state_.V = ((state_.A ^ inValue_) & (state_.A ^ result) & 0x80) != 0;

    state_.A = static_cast<uint8_t>(result);
    SetCompareFlags(result);
}

void Cpu::Sec()
{
    state_.C = true;
}

void Cpu::Sed()
{
    state_.D = true;
}

void Cpu::Sei()
{
    state_.I = true;

    // if the IRQ is already set it will still trigger for the next instruction
}

void Cpu::Shx()
{
    outValue_ = state_.X & ((address_ >> 8) + 1);
}

void Cpu::Shy()
{
    outValue_ = state_.Y & ((address_ >> 8) + 1);
}

void Cpu::Slo()
{
    state_.C = (inValue_ & 0x80) != 0;

    outValue_ = inValue_ << 1;
    state_.A |= outValue_;

    SetFlags(state_.A);
}

void Cpu::Sre()
{
    state_.C = (inValue_ & 0x01) != 0;

    outValue_ = inValue_ >> 1;

    state_.A ^= outValue_;
    SetFlags(state_.A);
}

void Cpu::Sta()
{
    Write(address_, state_.A);
}

void Cpu::StaZeroPage()
{
    WriteZeroPage(address_, state_.A);
}

void Cpu::Stp()
{
    // as we need to check this every cycle, we will overload the interrupt vector to do this.
    state_.InterruptVector = 1;
}

void Cpu::Stx()
{
    Write(address_, state_.X);
}

void Cpu::StxZeroPage()
{
    WriteZeroPage(address_, state_.X);
}

void Cpu::Sty()
{
    Write(address_, state_.Y);
}

void Cpu::StyZeroPage()
{
    WriteZeroPage(address_, state_.Y);
}

void Cpu::Tas()
{
    state_.S = state_.X & state_.A;
    outValue_ = ((address_ >> 8) + 1) & state_.S;
}

void Cpu::Tax()
{
    state_.X = state_.A;
    SetFlags(state_.X);
}

void Cpu::Tay()
{
    state_.Y = state_.A;
    SetFlags(state_.Y);
}

void Cpu::Tsx()
{
    state_.X = state_.S;
    SetFlags(state_.X);
}

void Cpu::Txa()
{
    state_.A = state_.X;
    SetFlags(state_.A);
}

void Cpu::Txs()
{
    state_.S = state_.X;
}

void Cpu::Tya()
{
    state_.A = state_.Y;
    SetFlags(state_.A);
}

void Cpu::Xaa()
{
    state_.A = state_.X & inValue_;
    SetFlags(state_.A);
}

void Cpu::Jump()
{
    bus_.CpuDummyRead(state_.PC);

    if ((state_.PC & 0xff00) != (address_ & 0xff00))
    {
        // page crossed
        bus_.CpuDummyRead((state_.PC & 0xff00) | (address_ & 0x00ff));
    }

    state_.PC = address_;
}

void Cpu::SetFlags(uint8_t value)
{
    state_.Z = value == 0;
    state_.N = (value & 0x80) != 0;
}

void Cpu::SetCompareFlags(uint16_t result)
{
    auto tmp = static_cast<uint8_t>(result);
    state_.N = (tmp & 0x80) != 0;
    state_.Z = tmp == 0;
    state_.C = (result & 0x100) == 0;
}

uint8_t Cpu::ReadProgramByte()
{
    return bus_.CpuReadProgramData(state_.PC++);
}

uint8_t Cpu::ReadData(uint16_t address)
{
    return bus_.CpuReadData(address);
}

uint8_t Cpu::ReadDataZeroPage(uint16_t address)
{
    return bus_.CpuReadZeroPage(address);
}

uint8_t Cpu::Pop()
{
    // technically not zero page, but page 1 works the same
    return bus_.CpuReadZeroPage((uint16_t)(0x100 + ++state_.S));
}

void Cpu::Write(uint16_t address, uint8_t value)
{
    bus_.CpuWrite(address, value);
}

void Cpu::WriteZeroPage(uint16_t address, uint8_t value)
{
    bus_.CpuWriteZeroPage(address, value);
}

void Cpu::Push(uint8_t value)
{
    // technically not zero page, but page 1 works the same
    bus_.CpuWriteZeroPage((uint16_t)(0x100 + state_.S--), value);
}

