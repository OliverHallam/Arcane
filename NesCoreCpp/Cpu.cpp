#include "Cpu.h"

#include "Bus.h"

Cpu::Cpu(Bus& bus) :
    bus_{ bus }
{
}

void Cpu::Reset()
{
    interruptVector_ = 0xfffc;
}

void Cpu::SetIrq(bool irq)
{
    irq_ = irq;
    if (irq_ && !I_ && interruptVector_ == 0)
        interruptVector_ = 0xfffe;
    else if (interruptVector_ == 0xfffe)
        interruptVector_ = 0;
}

void Cpu::SignalNmi()
{
    interruptVector_ = 0xfffa;
}

void Cpu::RunInstruction()
{
    if (interruptVector_ != 0)
    {
        if (skipInterrupt_)
        {
            skipInterrupt_ = false;
        }
        else
        {
            I_ = true;
            bus_.CpuDummyRead(PC_);
            bus_.CpuDummyRead(PC_);
            Interrupt();
            interruptVector_ = 0;
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
        Load();
        Ora();
        return;

    case 0x05:
        ZeroPage();
        Load();
        Ora();
        return;

    case 0x06:
        ZeroPage();
        Load();
        Asl();
        Store();
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

    case 0x10:
        Relative();
        Bpl();
        return;

    case 0x11:
        IndirectIndexRead();
        Load();
        Ora();
        return;

    case 0x15:
        ZeroPageX();
        Load();
        Ora();
        return;

    case 0x16:
        ZeroPageX();
        Load();
        Asl();
        Store();
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

    case 0x20:
        // timings are a little different on this one, so the decoding happens in the instruction
        Jsr();
        return;

    case 0x21:
        IndexIndirect();
        Load();
        And();
        return;

    case 0x24:
        ZeroPage();
        Load();
        Bit();
        return;

    case 0x25:
        ZeroPage();
        Load();
        And();
        return;

    case 0x26:
        ZeroPage();
        Load();
        Rol();
        Store();
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

    case 0x30:
        Relative();
        Bmi();
        return;

    case 0x31:
        IndirectIndexRead();
        Load();
        And();
        return;

    case 0x35:
        ZeroPageX();
        Load();
        And();
        return;

    case 0x36:
        ZeroPageX();
        Load();
        Rol();
        Store();
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

    case 0x40:
        Implicit();
        Rti();
        return;

    case 0x41:
        IndexIndirect();
        Load();
        Eor();
        return;

    case 0x45:
        ZeroPage();
        Load();
        Eor();
        return;

    case 0x46:
        ZeroPage();
        Load();
        Lsr();
        Store();
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

    case 0x50:
        Relative();
        Bvc();
        return;

    case 0x51:
        IndirectIndexRead();
        Load();
        Eor();
        return;

    case 0x55:
        ZeroPageX();
        Load();
        Eor();
        return;

    case 0x56:
        ZeroPageX();
        Load();
        Lsr();
        Store();
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

    case 0x60:
        Implicit();
        Rts();
        return;

    case 0x61:
        IndexIndirect();
        Load();
        Adc();
        return;

    case 0x65:
        ZeroPage();
        Load();
        Adc();
        return;

    case 0x66:
        ZeroPage();
        Load();
        Ror();
        Store();
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

    case 0x70:
        Relative();
        Bvs();
        return;

    case 0x71:
        IndirectIndexRead();
        Load();
        Adc();
        return;

    case 0x75:
        ZeroPageX();
        Load();
        Adc();
        return;

    case 0x76:
        ZeroPageX();
        Load();
        Ror();
        Store();
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

    case 0x81:
        IndexIndirect();
        Sta();
        return;

    case 0x84:
        ZeroPage();
        Sty();
        return;

    case 0x85:
        ZeroPage();
        Sta();
        return;

    case 0x86:
        ZeroPage();
        Stx();
        return;

    case 0x88:
        Implicit();
        Dey();
        return;

    case 0x8a:
        Implicit();
        Txa();
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

    case 0x90:
        Relative();
        Bcc();
        return;

    case 0x91:
        IndirectIndexWrite();
        Sta();
        return;

    case 0x94:
        ZeroPageX();
        Load();
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

    case 0x98:
        Implicit();
        Tya();
        return;

    case 0x99:
        AbsoluteYWrite();
        Load();
        Sta();
        return;

    case 0x9a:
        Implicit();
        Txs();
        return;

    case 0x9d:
        AbsoluteXWrite();
        Sta();
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

    case 0xa4:
        ZeroPage();
        Load();
        Ldy();
        return;

    case 0xa5:
        ZeroPage();
        Load();
        Lda();
        return;

    case 0xa6:
        ZeroPage();
        Load();
        Ldx();
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

    case 0xb0:
        Relative();
        Bcs();
        return;

    case 0xb1:
        IndirectIndexRead();
        Load();
        Lda();
        return;

    case 0xb4:
        ZeroPageX();
        Load();
        Ldy();
        return;

    case 0xb5:
        ZeroPageX();
        Load();
        Lda();
        return;

    case 0xb6:
        ZeroPageY();
        Load();
        Ldx();
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

    case 0xc0:
        Immediate();
        Cpy();
        return;

    case 0xc1:
        IndexIndirect();
        Load();
        Cmp();
        return;

    case 0xc4:
        ZeroPage();
        Load();
        Cpy();
        return;

    case 0xc5:
        ZeroPage();
        Load();
        Cmp();
        return;

    case 0xc6:
        ZeroPage();
        Load();
        Dec();
        Store();
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

    case 0xd0:
        Relative();
        Bne();
        return;

    case 0xd1:
        IndirectIndexRead();
        Load();
        Cmp();
        return;

    case 0xd5:
        ZeroPageX();
        Load();
        Cmp();
        return;

    case 0xd6:
        ZeroPageX();
        Load();
        Dec();
        Store();
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

    case 0xe0:
        Immediate();
        Cpx();
        return;

    case 0xe1:
        IndexIndirect();
        Load();
        Sbc();
        return;

    case 0xe4:
        ZeroPage();
        Load();
        Cpx();
        return;

    case 0xe5:
        ZeroPage();
        Load();
        Sbc();
        return;

    case 0xe6:
        ZeroPage();
        Load();
        Inc();
        Store();
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

    case 0xf0:
        Relative();
        Beq();
        return;

    case 0xf1:
        IndirectIndexRead();
        Load();
        Sbc();
        return;

    case 0xf5:
        ZeroPageX();
        Load();
        Sbc();
        return;

    case 0xf6:
        ZeroPageX();
        Load();
        Inc();
        Store();
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

    default:
        return;
    }
}

void Cpu::Interrupt()
{
    if (interruptVector_ != 0xfffc)
    {
        // don't push on reset
        Push((uint8_t)(PC_ >> 8));
        Push((uint8_t)(PC_ & 0xff));

        auto p = (uint8_t)(P() | 0x20);
        p |= (uint8_t)(B_ ? 0x10 : 0);
        Push(p);
    }
    else
    {
        bus_.CpuDummyRead(PC_);
        bus_.CpuDummyRead(PC_);
        bus_.CpuDummyRead(PC_);
    }

    auto pcLow = ReadData(interruptVector_);
    auto pcHigh = ReadData((uint16_t)(interruptVector_ + 1));

    PC_ = (uint16_t)((pcHigh << 8) | pcLow);
}

void Cpu::Jsr()
{
    auto lowByte = ReadProgramByte();

    bus_.CpuDummyRead(PC_);

    Push((uint8_t)(PC_ >> 8));
    Push((uint8_t)(PC_ & 0xff));

    auto highByte = ReadProgramByte();
    PC_ = (uint16_t)(highByte << 8 | lowByte);
}

void Cpu::Implicit()
{
    bus_.CpuDummyRead(PC_);
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
    address_ += X_;

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
    address_ += X_;
}

void Cpu::AbsoluteYRead()
{
    auto lowByte = ReadProgramByte();
    auto highByte = ReadProgramByte();

    address_ = (uint16_t)(lowByte | highByte << 8);
    address_ += Y_;

    if (highByte != (address_ >> 8))
    {
        bus_.CpuDummyRead(PC_ - 0x0100);
    }
}

void Cpu::AbsoluteYWrite()
{
    auto lowByte = ReadProgramByte();
    auto highByte = ReadProgramByte();

    address_ = (uint16_t)(lowByte | highByte << 8);
    address_ += Y_;
}

void Cpu::ZeroPage()
{
    address_ = ReadProgramByte();
}

void Cpu::ZeroPageX()
{
    auto zpAddress = ReadProgramByte();

    bus_.CpuDummyRead(zpAddress);

    address_ = (zpAddress + X_) & 0x00ff;
}

void Cpu::ZeroPageY()
{
    auto zpAddress = ReadProgramByte();

    bus_.CpuDummyRead(zpAddress);

    address_ = (zpAddress + Y_) & 0x00ff;
}

void Cpu::Relative()
{
    auto relative = (int8_t)ReadProgramByte();
    address_ = (uint16_t)(PC_ + relative);
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

    indirectAddress += X_;

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
    address_ += Y_;

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

    address_ += Y_;
}

void Cpu::Load()
{
    inValue_ = ReadData(address_);
}

void Cpu::Store()
{
    bus_.CpuWrite2(address_, inValue_, outValue_);
}

void Cpu::LoadA()
{
    inValue_ = A_;
}

void Cpu::StoreA()
{
    A_ = outValue_;
}

void Cpu::Adc()
{
    auto result = A_ + inValue_;

    if (C_)
    {
        result++;
    }

    V_ = (~(A_ ^ inValue_) & (A_ ^ result) & 0x80) != 0;
    C_ = (result & 0x100) != 0;

    A_ = (uint8_t)result;

    N_ = (A_ & 0x80) != 0;
    Z_ = A_ == 0;
}

void Cpu::And()
{
    A_ &= inValue_;
    SetFlags(A_);
}

void Cpu::Asl()
{
    C_ = (inValue_ & 0x80) != 0;

    outValue_ = inValue_ << 1;

    Z_ = outValue_ == 0;
    N_ = (outValue_ & 0x80) != 0;
}

void Cpu::Bcc()
{
    if (!C_)
        Jump();
}

void Cpu::Bcs()
{
    if (C_)
        Jump();
}

void Cpu::Beq()
{
    if (Z_)
        Jump();
}

void Cpu::Bit()
{
    N_ = (inValue_ & 0x80) != 0;
    V_ = (inValue_ & 0x40) != 0;
    Z_ = (inValue_ & A_) == 0;
}

void Cpu::Bmi()
{
    if (N_)
        Jump();
}

void Cpu::Bne()
{
    if (!Z_)
        Jump();
}

void Cpu::Bpl()
{
    if (!N_)
        Jump();
}

void Cpu::Brk()
{
    B_ = true;
    interruptVector_ = 0xfffe;
}

void Cpu::Bvc()
{
    if (!V_)
        Jump();
}

void Cpu::Bvs()
{
    if (V_)
        Jump();
}

void Cpu::Clc()
{
    C_ = false;
}

void Cpu::Cld()
{
    D_ = false;
}

void Cpu::Cli()
{
    I_ = false;
    if (irq_ && interruptVector_ == 0)
    {
        interruptVector_ = 0xfffe;
        skipInterrupt_ = true;
    }
}

void Cpu::Clv()
{
    V_ = false;
}

void Cpu::Cmp()
{
    SetCompareFlags(A_);
}

void Cpu::Cpx()
{
    SetCompareFlags(X_);
}

void Cpu::Cpy()
{
    SetCompareFlags(Y_);
}

void Cpu::Dec()
{
    outValue_ = inValue_ - 1;
    SetFlags(outValue_);
}

void Cpu::Dex()
{
    X_--;
    SetFlags(X_);
}

void Cpu::Dey()
{
    Y_--;
    SetFlags(Y_);
}

void Cpu::Eor()
{
    A_ ^= inValue_;
    SetFlags(A_);
}

void Cpu::Inc()
{
    outValue_ = inValue_ + 1;
    SetFlags(outValue_);
}

void Cpu::Inx()
{
    X_++;
    SetFlags(X_);
}

void Cpu::Iny()
{
    Y_++;
    SetFlags(Y_);
}

void Cpu::Jmp()
{
    PC_ = address_;
}

void Cpu::Lda()
{
    A_ = inValue_;
    SetFlags(A_);
}

void Cpu::Ldx()
{
    X_ = inValue_;
    SetFlags(X_);
}

void Cpu::Ldy()
{
    Y_ = inValue_;
    SetFlags(Y_);
}

void Cpu::Lsr()
{
    C_ = (inValue_ & 0x01) != 0;

    outValue_ = inValue_ >> 1;

    N_ = false; // top bit is left unset
    Z_ = outValue_ == 0;
}

void Cpu::Ora()
{
    A_ |= inValue_;
    SetFlags(A_);
}

void Cpu::Nop()
{
}

void Cpu::Pha()
{
    Push(A_);
}

void Cpu::Php()
{
    Push(P() | 0x30);
}

void Cpu::Pla()
{
    bus_.CpuDummyRead(PC_);

    A_ = Pop();

    SetFlags(A_);
}

void Cpu::Plp()
{
    bus_.CpuDummyRead(PC_);

    P(Pop());

    if (!I_ && irq_ && interruptVector_ == 0)
    {
        interruptVector_ = 0xfffe;
        skipInterrupt_ = true;
    }
}

void Cpu::Rol()
{
    auto carry = (inValue_ & 0x80) != 0;

    outValue_ = inValue_ << 1;

    if (C_)
        outValue_ |= 0x01;

    Z_ = outValue_ == 0;
    N_ = (outValue_ & 0x80) != 0;
    C_ = carry;
}

void Cpu::Ror()
{
    auto carry = (inValue_ & 1) != 0;

    outValue_ = inValue_ >> 1;

    if (C_)
        outValue_ |= 0x80;

    Z_ = outValue_ == 0;
    N_ = (outValue_ & 0x80) != 0;
    C_ = carry;
}

void Cpu::Rti()
{
    bus_.CpuDummyRead(PC_);

    P(Pop());
    auto pcLow = Pop();
    auto pcHigh = Pop();

    PC_ = (uint16_t)(pcHigh << 8 | pcLow);

    if (!I_ && irq_ && interruptVector_ == 0)
    {
        interruptVector_ = 0xfffe;
    }
}

void Cpu::Rts()
{
    auto lowByte = Pop();
    auto highByte = Pop();

    bus_.CpuDummyRead(PC_);
    bus_.CpuDummyRead(PC_);

    PC_ = (uint16_t)(((highByte << 8) | lowByte) + 1);
}

void Cpu::Sec()
{
    C_ = true;
}

void Cpu::Sed()
{
    D_ = true;
}

void Cpu::Sei()
{
    I_ = true;

    // if the IRQ is already set it will still trigger for the next instruction
}

void Cpu::Sbc()
{
    auto result = (uint16_t)A_ - inValue_;
    if (!C_)
    {
        result--;
    }

    V_ = ((A_ ^ inValue_) & (A_ ^ result) & 0x80) != 0;
    C_ = (result & 0x100) == 0;

    A_ = (uint8_t)result;

    N_ = (A_ & 0x80) != 0;
    Z_ = A_ == 0;
}

void Cpu::Sta()
{
    Write(address_, A_);
}

void Cpu::Stx()
{
    Write(address_, X_);
}

void Cpu::Sty()
{
    Write(address_, Y_);
}

void Cpu::Tax()
{
    X_ = A_;
    SetFlags(X_);
}

void Cpu::Tay()
{
    Y_ = A_;
    SetFlags(Y_);
}

void Cpu::Tsx()
{
    X_ = S_;
    SetFlags(X_);
}

void Cpu::Txa()
{
    A_ = X_;
    SetFlags(A_);
}

void Cpu::Txs()
{
    S_ = X_;
}

void Cpu::Tya()
{
    A_ = Y_;
    SetFlags(A_);
}

void Cpu::Jump()
{
    bus_.CpuDummyRead(PC_);

    if ((PC_ & 0xff00) != (address_ & 0xff00))
    {
        // page crossed
        bus_.CpuDummyRead((PC_ & 0xff00) | (address_ & 0x00ff));
    }

    PC_ = address_;
}

void Cpu::SetFlags(uint8_t value)
{
    Z_ = value == 0;
    N_ = (value & 0x80) != 0;
}

void Cpu::SetCompareFlags(uint8_t reg)
{
    auto result = reg - inValue_;

    auto tmp = (uint8_t)result;

    N_ = (tmp & 0x80) != 0;
    Z_ = tmp == 0;
    C_ = (result & 0x100) == 0;
}

uint8_t Cpu::ReadProgramByte()
{
    return bus_.CpuReadProgramData(PC_++);
}

uint8_t Cpu::ReadData(uint16_t address)
{
    return bus_.CpuReadData(address);
}

uint8_t Cpu::Pop()
{
    return bus_.CpuReadData((uint16_t)(0x100 + ++S_));
}

void Cpu::Write(uint16_t address, uint8_t value)
{
    bus_.CpuWrite(address, value);
}

void Cpu::Push(uint8_t value)
{
    bus_.CpuWrite((uint16_t)(0x100 + S_--), value);
}

uint8_t Cpu::P()
{
    return
        (N_ ? 0x80 : 0) |
        (V_ ? 0x40 : 0) |
        (D_ ? 0x08 : 0) |
        (I_ ? 0x04 : 0) |
        (Z_ ? 0x02 : 0) |
        (C_ ? 0x01 : 0);
}

void Cpu::P(uint8_t value)
{
    N_ = (value & 0x80) != 0;
    V_ = (value & 0x40) != 0;
    D_ = (value & 0x08) != 0;
    I_ = (value & 0x04) != 0;
    Z_ = (value & 0x02) != 0;
    C_ = (value & 0x01) != 0;
}
