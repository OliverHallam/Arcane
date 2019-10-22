﻿namespace NesEmu.Emulator
{
    public class NesCpu
    {
        private Bus bus;

        // Registers
        public byte A;
        public byte X;
        public byte Y;
        public ushort PC;
        public byte S;

        // the flags register as seperate bytes
        private bool C, Z, I, D, B, V, N;
        public byte P
        {
            get
            {
                return (byte)
                    ((this.N ? 0x80 : 0) |
                    (this.V ? 0x40 : 0) |
                    (this.B ? 0x10 : 0) |
                    (this.D ? 0x08 : 0) |
                    (this.I ? 0x04 : 0) |
                    (this.Z ? 0x02 : 0) |
                    (this.C ? 0x01 : 0));

            }
            set
            {
                this.N = (value & 0x80) != 0;
                this.V = (value & 0x40) != 0;
                this.B = (value & 0x10) != 0;
                this.D = (value & 0x08) != 0;
                this.I = (value & 0x04) != 0;
                this.Z = (value & 0x02) != 0;
                this.C = (value & 0x01) != 0;
            }
        }

        private ushort address;
        private byte value;

        private bool nmi;

        public NesCpu(Bus bus)
        {
            this.bus = bus;
        }

        public void Reset()
        {
            // 7 cycles
            this.C = this.Z = this.D = this.B = this.V = this.N = false;
            this.I = true;

            this.PC = (ushort)(bus.CpuRead(0xfffd) << 8 | bus.CpuRead(0xfffc));
        }

        public void SignalNmi()
        {
            this.nmi = true;
        }

        public void RunInstruction()
        {
            if (this.nmi)
            {
                this.nmi = false;
                this.bus.TickCpu();
                this.bus.TickCpu();
                this.Nmi();
                return;
            }

            var opCode = bus.CpuRead(this.PC++);
            this.bus.TickCpu();

            switch(opCode)
            {
                case 0x05:
                    this.ZeroPage();
                    this.Load();
                    this.Ora();
                    return;

                case 0x06:
                    this.ZeroPage();
                    this.Load();
                    this.Asl();
                    this.Store();
                    return;

                case 0x09:
                    this.Immediate();
                    this.Ora();
                    return;

                case 0x0a:
                    this.Implicit();
                    this.LoadA();
                    this.Asl();
                    this.StoreA();
                    return;

                case 0x10:
                    this.Relative();
                    this.Bpl();
                    return;

                case 0x18:
                    this.Implicit();
                    this.Clc();
                    return;

                case 0x20:
                    // timings are a little different on this one, so the decoding happens in the instruction
                    this.Jsr();
                    return;

                case 0x24:
                    this.ZeroPage();
                    this.Load();
                    this.Bit();
                    return;

                case 0x25:
                    this.ZeroPage();
                    this.Load();
                    this.And();
                    return;

                case 0x26:
                    this.ZeroPage();
                    this.Load();
                    this.Rol();
                    this.Store();
                    return;

                case 0x29:
                    this.Immediate();
                    this.And();
                    return;

                case 0x2a:
                    this.Implicit();
                    this.LoadA();
                    this.Rol();
                    this.StoreA();
                    return;

                case 0x30:
                    this.Relative();
                    this.Bmi();
                    return;

                case 0x35:
                    this.ZeroPageX();
                    this.Load();
                    this.And();
                    return;

                case 0x38:
                    this.Implicit();
                    this.Sec();
                    return;

                case 0x40:
                    this.Implicit();
                    this.Rti();
                    return;

                case 0x45:
                    this.ZeroPage();
                    this.Load();
                    this.Eor();
                    return;

                case 0x46:
                    this.ZeroPage();
                    this.Lsr();
                    return;

                case 0x48:
                    this.bus.TickCpu();
                    this.Pha();
                    return;

                case 0x49:
                    this.Immediate();
                    this.Eor();
                    return;

                case 0x4a:
                    this.Implicit();
                    this.LoadA();
                    this.Lsr();
                    this.StoreA();
                    return;

                case 0x4c:
                    this.Absolute();
                    this.Jmp();
                    return;

                case 0x50:
                    this.Relative();
                    this.Bvc();
                    return;

                case 0x60:
                    this.Implicit();
                    this.Rts();
                    return;

                case 0x65:
                    this.ZeroPage();
                    this.Load();
                    this.Adc();
                    return;

                case 0x66:
                    this.ZeroPage();
                    this.Load();
                    this.Ror();
                    this.Store();
                    return;

                case 0x68:
                    this.Implicit();
                    this.Pla();
                    return;

                case 0x69:
                    this.Immediate();
                    this.Adc();
                    return;

                case 0x6a:
                    this.Implicit();
                    this.LoadA();
                    this.Ror();
                    this.StoreA();
                    return;

                case 0x6c:
                    this.AbsoluteIndirect();
                    this.Jmp();
                    return;

                case 0x6d:
                    this.Absolute();
                    this.Load();
                    this.Adc();
                    return;

                case 0x70:
                    this.Relative();
                    this.Bvs();
                    return;

                case 0x78:
                    this.Implicit();
                    this.Sei();
                    return;

                case 0x84:
                    this.ZeroPage();
                    this.Sty();
                    return;

                case 0x85:
                    this.ZeroPage();
                    this.Sta();
                    return;

                case 0x86:
                    this.ZeroPage();
                    this.Stx();
                    return;

                case 0x88:
                    this.Implicit();
                    this.Dey();
                    return;

                case 0x8a:
                    this.Implicit();
                    this.Txa();
                    return;

                case 0x8c:
                    this.Absolute();
                    this.Sty();
                    return;

                case 0x8d:
                    this.Absolute();
                    this.Sta();
                    return;

                case 0x8e:
                    this.Absolute();
                    this.Stx();
                    return;

                case 0x90:
                    this.Relative();
                    this.Bcc();
                    return;

                case 0x91:
                    this.IndirectIndexed();
                    this.Sta();
                    return;

                case 0x95:
                    this.ZeroPageX();
                    this.Sta();
                    return;

                case 0x98:
                    this.Implicit();
                    this.Tya();
                    return;

                case 0x9a:
                    this.Implicit();
                    this.Txs();
                    return;

                case 0x9d:
                    this.AbsoluteX();
                    this.Sta();
                    return;

                case 0xa0:
                    this.Immediate();
                    this.Ldy();
                    break;

                case 0xa2:
                    this.Immediate();
                    this.Ldx();
                    return;

                case 0xa4:
                    this.ZeroPage();
                    this.Load();
                    this.Ldy();
                    return;

                case 0xa5:
                    this.ZeroPage();
                    this.Load();
                    this.Lda();
                    return;

                case 0xa6:
                    this.ZeroPage();
                    this.Load();
                    this.Ldx();
                    return;

                case 0xa8:
                    this.Implicit();
                    this.Tay();
                    return;

                case 0xa9:
                    this.Immediate();
                    this.Lda();
                    return;

                case 0xaa:
                    this.Implicit();
                    this.Tax();
                    return;

                case 0xac:
                    this.Absolute();
                    this.Load();
                    this.Ldy();
                    return;

                case 0xad:
                    this.Absolute();
                    this.Load();
                    this.Lda();
                    return;

                case 0xae:
                    this.Absolute();
                    this.Load();
                    this.Ldx();
                    return;

                case 0xb0:
                    this.Relative();
                    this.Bcs();
                    return;

                case 0xb1:
                    this.IndirectIndexed();
                    this.Load();
                    this.Lda();
                    return;

                case 0xb4:
                    this.ZeroPageX();
                    this.Load();
                    this.Ldy();
                    return;

                case 0xb5:
                    this.ZeroPageX();
                    this.Load();
                    this.Lda();
                    return;

                case 0xb9:
                    this.AbsoluteY();
                    this.Load();
                    this.Lda();
                    return;

                case 0xbd:
                    this.AbsoluteX();
                    this.Load();
                    this.Lda();
                    return;

                case 0xc0:
                    this.Immediate();
                    this.Cpy();
                    return;

                case 0xc5:
                    this.ZeroPage();
                    this.Load();
                    this.Cmp();
                    return;

                case 0xc6:
                    this.ZeroPage();
                    this.Load();
                    this.Dec();
                    this.Store();
                    return;

                case 0xc8:
                    this.Implicit();
                    this.Iny();
                    return;

                case 0xc9:
                    this.Immediate();
                    this.Cmp();
                    return;

                case 0xca:
                    this.Implicit();
                    this.Dex();
                    return;

                case 0xce:
                    this.Absolute();
                    this.Load();
                    this.Dec();
                    this.Store();
                    return;

                case 0xd0:
                    this.Relative();
                    this.Bne();
                    return;

                case 0xd6:
                    this.ZeroPageX();
                    this.Load();
                    this.Dec();
                    this.Store();
                    return;

                case 0xd8:
                    this.Implicit();
                    this.Cld();
                    return;

                case 0xe0:
                    this.Immediate();
                    this.Cpx();
                    return;

                case 0xe5:
                    this.ZeroPage();
                    this.Load();
                    this.Sbc();
                    return;

                case 0xe6:
                    this.ZeroPage();
                    this.Load();
                    this.Inc();
                    this.Store();
                    return;

                case 0xe8:
                    this.Implicit();
                    this.Inx();
                    return;

                case 0xe9:
                    this.Immediate();
                    this.Sbc();
                    return;

                case 0xea:
                    this.Implicit();
                    this.Nop();
                    return;

                case 0xee:
                    this.Absolute();
                    this.Load();
                    this.Inc();
                    this.Store();
                    return;

                case 0xf0:
                    this.Relative();
                    this.Beq();
                    return;

                default:
                    return;
            }
        }

        private void Jsr()
        {
            var lowByte = bus.CpuRead(this.PC++);
            this.bus.TickCpu();

            this.bus.TickCpu();

            this.bus.TickCpu();
            this.bus.CpuWrite((ushort)(0x100 + this.S--), (byte)(this.PC >> 8));

            this.bus.TickCpu();
            this.bus.CpuWrite((ushort)(0x100 + this.S--), (byte)(this.PC & 0xff));

            var highByte = this.bus.CpuRead(this.PC);
            this.PC = (ushort)(highByte << 8 | lowByte);
            this.bus.TickCpu();
        }

        public void Implicit()
        {
            this.bus.TickCpu();
        }

        private void Immediate()
        {
            this.value = bus.CpuRead(this.PC++);
            this.bus.TickCpu();
        }

        private void Absolute()
        {
            var lowByte = this.bus.CpuRead(this.PC++);
            this.bus.TickCpu();

            var highByte = this.bus.CpuRead(this.PC++);
            this.bus.TickCpu();

            this.address = (ushort)(lowByte | highByte << 8);
        }

        private void AbsoluteX()
        {
            var lowByte = this.bus.CpuRead(this.PC++);
            this.bus.TickCpu();

            var highByte = this.bus.CpuRead(this.PC++);
            this.bus.TickCpu();

            this.address = (ushort)(lowByte | highByte << 8);
            this.address += this.X;

            var addressHigh = this.address >> 8;

            if (addressHigh != (this.address >> 8))
            {
                this.bus.TickCpu();
            }
        }

        private void AbsoluteY()
        {
            var lowByte = this.bus.CpuRead(this.PC++);
            this.bus.TickCpu();

            var highByte = this.bus.CpuRead(this.PC++);
            this.bus.TickCpu();

            this.address = (ushort)(lowByte | highByte << 8);
            this.address += this.Y;

            var addressHigh = this.address >> 8;

            if (addressHigh != (this.address >> 8))
            {
                this.bus.TickCpu();
            }
        }

        private void ZeroPage()
        {
            this.address = bus.CpuRead(this.PC++);
            this.bus.TickCpu();
        }

        private void ZeroPageX()
        {
            this.address = bus.CpuRead(this.PC++);
            this.bus.TickCpu();

            this.bus.TickCpu();
            this.address += this.X;
        }

        private void Relative()
        {
            var relative = (sbyte)(this.bus.CpuRead(this.PC++));
            this.address = (ushort)(this.PC + relative);
            this.bus.TickCpu();
        }

        private void AbsoluteIndirect()
        {
            var addressLow = this.bus.CpuRead(this.PC++);
            this.bus.TickCpu();

            var addressHigh = this.bus.CpuRead(this.PC++);
            this.bus.TickCpu();

            var address = (ushort)((addressHigh << 8) | addressLow);
            
            addressLow++;
            var nextAddress = (ushort)((addressHigh << 8) | addressLow);

            addressLow = this.bus.CpuRead(address);
            this.bus.TickCpu();

            addressHigh = this.bus.CpuRead(nextAddress);

            this.address = (ushort)((addressHigh << 8) | addressLow);
        }

        private void IndirectIndexed()
        {
            ushort indirectAddress = bus.CpuRead(this.PC++);
            this.bus.TickCpu();

            var addressLow = this.bus.CpuRead(indirectAddress);
            this.bus.TickCpu();

            var addressHigh = this.bus.CpuRead((ushort)(indirectAddress + 1));
            this.bus.TickCpu();

            this.address = (ushort)(addressLow | addressHigh << 8);
            this.address += this.Y;
            this.bus.TickCpu();
        }

        private void Load()
        {
            this.value = bus.CpuRead(this.address);
            this.bus.TickCpu();
        }

        private void Store()
        {
            this.bus.TickCpu();

            this.bus.TickCpu();
            this.bus.CpuWrite(this.address, this.value);
        }

        private void LoadA()
        {
            this.value = this.A;
        }

        private void StoreA()
        {
            this.bus.TickCpu();
            this.A = this.value;
        }

        private void Adc()
        {
            var highBit = this.A & 0x80;

            var result = this.A + this.value;

            if (this.C)
            {
                result++;
            }

            this.A = (byte)result;

            this.N = (this.A & 0x80) != 0;
            this.Z = this.A == 0;
            this.C = (result & 0x100) != 0;
            this.V = (this.A & 0x80) != highBit;
        }

        private void And()
        {
            this.A &= this.value;
            this.SetFlags(this.A);
        }

        private void Asl()
        {
            this.C = (this.value & 0x80) != 0;

            this.value <<= 1;

            this.Z = this.value == 0;
            this.N = (this.value & 0x80) != 0;
        }

        private void Bcc()
        {
            if (!this.C)
            {
                this.Jump();
            }
        }

        private void Bcs()
        {
            if (this.C)
            {
                this.Jump();
            }
        }

        private void Beq()
        {
            if (this.Z)
            {
                this.Jump();
            }
        }

        private void Bit()
        {
            this.N = (this.value & 0x80) != 0;
            this.V = (this.value & 0x70) != 0;
            this.Z = (this.value & this.A) == 0;
        }

        private void Bmi()
        {
            if (this.N)
            {
                this.Jump();
            }
        }

        private void Bne()
        {
            if (!this.Z)
            {
                this.Jump();
            }
        }

        private void Bpl()
        {
            if (!this.N)
            {
                this.Jump();
            }
        }

        private void Bvc()
        {
            if (!this.V)
            {
                this.Jump();
            }
        }

        private void Bvs()
        {
            if (this.V)
            {
                this.Jump();
            }
        }

        private void Clc()
        {
            this.C = false;
        }

        private void Cld()
        {
            this.D = false;
        }

        private void Cmp()
        {
            SetCompareFlags(this.A);
        }

        private void Cpx()
        {
            SetCompareFlags(this.X);
        }

        private void Cpy()
        {
            SetCompareFlags(this.X);
        }

        private void Dec()
        {
            this.value--;
            this.SetFlags(this.value);
        }

        private void Dex()
        {
            this.X--;
            this.SetFlags(this.X);
        }

        private void Dey()
        {
            this.Y--;
            this.SetFlags(this.Y);
        }

        private void Eor()
        {
            this.A ^= this.value;
            this.SetFlags(this.A);
        }

        private void Inc()
        {
            value++;
            this.SetFlags(value);
        }

        private void Inx()
        {
            this.X++;
            this.SetFlags(this.X);
        }

        private void Iny()
        {
            this.Y++;
            this.SetFlags(this.Y);
        }

        private void Jmp()
        {
            this.PC = address;
        }

        private void Lda()
        {
            this.A = value;
            this.SetFlags(this.A);
        }

        private void Ldx()
        {
            this.X = value;
            this.SetFlags(this.X);
        }

        private void Ldy()
        {
            this.Y = value;
            this.SetFlags(this.Y);
        }

        private void Lsr()
        {
            this.C = (this.value & 0x01) != 0;

            this.value >>= 1;

            this.N = false; // top bit is left unset
            this.Z = this.value == 0;
        }

        private void Nmi()
        {
            this.bus.TickCpu();
            this.bus.CpuWrite((ushort)(0x100 + this.S--), (byte)(this.PC >> 8));

            this.bus.TickCpu();
            this.bus.CpuWrite((ushort)(0x100 + this.S--), (byte)(this.PC & 0xff));

            this.bus.TickCpu();
            // set the B flag on the stack
            this.bus.CpuWrite((ushort)(0x100 + this.S--), (byte)(this.P | 0x10));

            var pcLow = this.bus.CpuRead(0xfffa);
            this.bus.TickCpu();

            var pcHigh = this.bus.CpuRead(0xfffb);
            this.bus.TickCpu();

            this.PC = (ushort)((pcHigh << 8) | pcLow);
        }

        private void Ora()
        {
            this.A |= this.value;
            this.SetFlags(this.A);
        }

        private void Nop()
        {
        }

        private void Pha()
        {
            this.bus.TickCpu();
            this.bus.CpuWrite((ushort)(0x100 + this.S--), this.A);
        }

        private void Pla()
        {
            ++this.S;
            this.bus.TickCpu();

            this.A = this.bus.CpuRead((ushort)(0x100 + this.S));
            this.bus.TickCpu();
        }

        private void Rol()
        {
            var carry = (this.value & 0x80) != 0;

            this.value <<= 1;

            if (this.C)
            {
                this.value |= 0x01;
            }

            this.Z = this.value == 0;
            this.N = (this.value & 0x80) != 0;
            this.C = carry;
        }

        private void Ror()
        {
            var carry = (this.value & 1) != 0;

            this.value >>= 1;

            if (this.C)
            {
                this.value |= 0x80;
            }

            this.Z = this.value == 0;
            this.N = (this.value & 0x80) != 0;
            this.C = carry;
        }


        private void Rti()
        {
            this.bus.TickCpu();

            this.P = this.bus.CpuRead((ushort)(0x100 + ++this.S));
            this.bus.TickCpu();

            var pcLow = this.bus.CpuRead((ushort)(0x100 + ++this.S));
            this.bus.TickCpu();

            var pcHigh = this.bus.CpuRead((ushort)(0x100 + ++this.S));
            this.bus.TickCpu();

            this.PC = (ushort)(pcHigh << 8 | pcLow);
        }

        private void Rts()
        {
            this.bus.TickCpu();

            var lowByte = this.bus.CpuRead((ushort)(0x100 + ++this.S));
            this.bus.TickCpu();

            var highByte = this.bus.CpuRead((ushort)(0x100 + ++this.S));
            this.bus.TickCpu();

            this.PC = (ushort)((highByte << 8) | lowByte + 1);
            this.bus.TickCpu();
        }

        private void Sec()
        {
            this.C = true;
        }

        private void Sei()
        {
            this.I = true;
        }

        private void Sbc()
        {
            var highBit = (this.A & 0x80);

            var result = (int)this.A - value;
            if (!this.C)
            {
                result--;
            }

            this.A = (byte)result;

            this.N = (this.A & 0x80) != 0;
            this.Z = this.A == 0;
            this.C = (result & 0x100) == 0;
            this.V = (this.A & 0x80) != highBit;
        }

        private void Sta()
        {
            this.bus.TickCpu();
            this.bus.CpuWrite(this.address, this.A);
        }

        private void Stx()
        {
            this.bus.TickCpu();
            this.bus.CpuWrite(this.address, this.X);
        }

        private void Sty()
        {
            this.bus.TickCpu();
            this.bus.CpuWrite(this.address, this.Y);
        }

        private void Tax()
        {
            this.X = this.A;
            this.SetFlags(this.X);
        }

        private void Tay()
        {
            this.Y = this.A;
            this.SetFlags(this.Y);
        }

        private void Txa()
        {
            this.A = this.X;
            this.SetFlags(this.A);
        }

        private void Txs()
        {
            this.S = this.X;
        }

        private void Tya()
        {
            this.A = this.Y;
            this.SetFlags(this.A);
        }

        private void Jump()
        {
            if ((this.PC & 0xff00) != (this.address & 0xff00))
            {
                // page crossed
                this.bus.TickCpu();
            }

            this.PC = this.address;
        }

        private void SetFlags(byte a)
        {
            this.Z = a == 0;
            this.N = (a & 0x80) != 0;
        }

        private void SetCompareFlags(byte reg)
        {
            var result = reg - this.value;

            var tmp = (byte)result;

            this.N = (tmp & 0x80) != 0;
            this.Z = tmp == 0;
            this.C = (result & 0x100) == 0;
        }
    }
}
