using System;
using System.Collections.Generic;
using System.Text;

namespace NesEmu.Emulator
{
    public class NesCpu
    {
        private const byte CFlag = 0x01;
        private const byte ZFlag = 0x02;
        private const byte BFlag = 0x08;
        private const byte NFlag = 0x80;

        private Bus bus;

        // Registers
        public byte A;
        public byte X;
        public byte Y;
        public ushort PC;
        public byte S;
        public byte P;

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
            this.P = 4;
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
                    this.Asl();
                    return;

                case 0x09:
                    this.Immediate();
                    this.Ora();
                    return;

                case 0x0a:
                    this.bus.TickCpu();
                    this.AslA();
                    return;

                case 0x10:
                    this.Relative();
                    this.Bpl();
                    return;

                case 0x18:
                    this.bus.TickCpu();
                    this.Clc();
                    return;

                case 0x20:
                    // timings are a little different on this one, so the decoding happens in the instruction
                    this.Jsr();
                    return;

                case 0x25:
                    this.ZeroPage();
                    this.Load();
                    this.And();
                    return;

                case 0x26:
                    this.ZeroPage();
                    this.Rol();
                    return;

                case 0x29:
                    this.Immediate();
                    this.And();
                    return;

                case 0x2a:
                    this.bus.TickCpu();
                    this.RolA();
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
                    this.bus.TickCpu();
                    this.Sec();
                    return;

                case 0x40:
                    this.bus.TickCpu();
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
                    this.bus.TickCpu();
                    this.LsrA();
                    return;

                case 0x4c:
                    this.Absolute();
                    this.Jmp();
                    return;

                case 0x60:
                    this.bus.TickCpu();
                    this.Rts();
                    return;

                case 0x65:
                    this.ZeroPage();
                    this.Load();
                    this.Adc();
                    return;

                case 0x66:
                    this.ZeroPage();
                    this.Ror();
                    return;

                case 0x68:
                    this.bus.TickCpu();
                    this.Pla();
                    return;

                case 0x69:
                    this.Immediate();
                    this.Adc();
                    return;

                case 0x6a:
                    this.bus.TickCpu();
                    this.RorA();
                    return;

                case 0x6d:
                    this.Absolute();
                    this.Adc();
                    return;

                case 0x78:
                    this.bus.TickCpu();
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
                    this.bus.TickCpu();
                    this.Dey();
                    return;

                case 0x8a:
                    this.bus.TickCpu();
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
                    this.Load();
                    this.Sta();
                    return;

                case 0x98:
                    this.bus.TickCpu();
                    this.Tya();
                    return;

                case 0x9a:
                    this.bus.TickCpu();
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
                    this.bus.TickCpu();
                    this.Tay();
                    return;

                case 0xa9:
                    this.Immediate();
                    this.Lda();
                    return;

                case 0xaa:
                    this.bus.TickCpu();
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
                    this.Dec();
                    return;

                case 0xc8:
                    this.bus.TickCpu();
                    this.Iny();
                    return;

                case 0xc9:
                    this.Immediate();
                    this.Cmp();
                    return;

                case 0xca:
                    this.bus.TickCpu();
                    this.Dex();
                    return;

                case 0xce:
                    this.Absolute();
                    this.Dec();
                    return;

                case 0xd0:
                    this.Relative();
                    this.Bne();
                    return;

                case 0xd6:
                    this.ZeroPageX();
                    this.Dec();
                    return;

                case 0xd8:
                    this.bus.TickCpu();
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
                    this.Inc();
                    return;

                case 0xe8:
                    this.bus.TickCpu();
                    this.Inx();
                    return;

                case 0xe9:
                    this.Immediate();
                    this.Sbc();
                    return;

                case 0xee:
                    this.Absolute();
                    this.Inc();
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

        private void Adc()
        {
            var result = this.A + this.value;

            if ((this.P & CFlag) != 0)
            {
                result++;
            }

            this.A = (byte)(result & 0xff);

            this.P &= 0x7c;

            if ((this.A & 0x08) != 0)
            {
                this.P |= NFlag;
            }

            if (this.A == 0)
            {
                this.P |= ZFlag;
            }

            this.P |= (byte)(result >> 8);
        }

        private void And()
        {
            this.A &= this.value;
            this.SetFlags(this.A);
        }

        private void Asl()
        {
            var value = this.bus.CpuRead(this.address);
            this.bus.TickCpu();

            var highBit = value & 0x80;

            value <<= 1;

            this.P &= 0x7c;

            if (value == 0)
            {
                this.P |= ZFlag;
            }

            if ((value & 0x80) != 0)
            {
                this.P |= NFlag;
            }

            this.P |= (byte)(highBit >> 7);

            this.bus.TickCpu();

            this.bus.TickCpu();
            this.bus.CpuWrite(this.address, value);
        }

        private void AslA()
        {
            this.bus.TickCpu();

            var highBit = this.A & 0x80;

            this.A <<= 1;

            this.P &= 0x7c;

            if (this.A == 0)
            {
                this.P |= ZFlag;
            }

            if ((this.A & 0x80) != 0)
            {
                this.P |= NFlag;
            }

            this.P |= (byte)(highBit >> 7);
        }

        private void Bcc()
        {
            if ((this.P & CFlag) == 0)
            {
                this.Jump();
            }
        }

        private void Bcs()
        {
            if ((this.P & CFlag) != 0)
            {
                this.Jump();
            }
        }

        private void Beq()
        {
            if ((this.P & ZFlag) != 0)
            {
                this.Jump();
            }
        }

        private void Bmi()
        {
            if ((this.P & NFlag) != 0)
            {
                this.Jump();
            }
        }

        private void Bne()
        {
            if ((this.P & ZFlag) == 0)
            {
                this.Jump();
            }
        }

        private void Bpl()
        {
            if ((this.P & NFlag) == 0)
            {
                this.Jump();
            }
        }

        private void Clc()
        {
            this.P &= 0xfe;
        }

        private void Cld()
        {
            this.P &= 0xf7;
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
            var value = this.bus.CpuRead(this.address);
            this.bus.TickCpu();

            value--;

            this.bus.TickCpu();
            this.bus.CpuWrite(this.address, value);
            
            this.bus.TickCpu();
            this.SetFlags(value);
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
            var value = this.bus.CpuRead(this.address);
            this.bus.TickCpu();

            value++;

            this.bus.TickCpu();

            this.bus.TickCpu();
            this.bus.CpuWrite(this.address, value);
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

        private void LsrA()
        {
            var lowBit = A & 1;

            this.A >>= 1;

            this.P &= 0x7c;

            if (this.A == 0)
            {
                this.P |= ZFlag;
            }

            this.P |= (byte)lowBit;
        }

        private void Lsr()
        {
            var value = this.bus.CpuRead(this.address);
            this.bus.TickCpu();

            var lowBit = value & 0x80;

            value >>= 1;

            this.P &= 0x7c;

            if (value == 0)
            {
                this.P |= ZFlag;
            }

            this.P |= (byte)lowBit;

            this.bus.TickCpu();

            this.bus.TickCpu();
            this.bus.CpuWrite(this.address, value);
        }

        private void Nmi()
        {
            this.bus.TickCpu();
            this.bus.CpuWrite((ushort)(0x100 + this.S--), (byte)(this.PC >> 8));

            this.bus.TickCpu();
            this.bus.CpuWrite((ushort)(0x100 + this.S--), (byte)(this.PC & 0xff));

            this.bus.TickCpu();
            this.bus.CpuWrite((ushort)(0x100 + this.S--), (byte)(this.P | BFlag));

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
            var value = this.bus.CpuRead(this.address);
            this.bus.TickCpu();

            var highBit = value & 0x80;
            value <<= 1;
            if ((this.P & CFlag) != 0)
            {
                value |= 0x01;
            }

            this.P &= 0x7c;

            if (value == 0)
            {
                this.P |= ZFlag;
            }

            if ((value & 0x80) != 0)
            {
                this.P |= NFlag;
            }

            this.P |= (byte)(highBit >> 7);

            this.bus.TickCpu();

            this.bus.TickCpu();
            this.bus.CpuWrite(this.address, value);
        }

        private void RolA()
        {
            var highBit = this.A & 0x80;
            this.A <<= 1;
            if ((this.P & CFlag) != 0)
            {
                this.A |= 0x01;
            }

            this.P &= 0x7c;

            if (this.A == 0)
            {
                this.P |= ZFlag;
            }

            if ((this.A & 0x80) != 0)
            {
                this.P |= NFlag;
            }

            this.P |= (byte)(highBit >> 7);
        }

        private void Ror()
        {
            var value = this.bus.CpuRead(this.address);
            this.bus.TickCpu();

            var lowBit = value & 1;
            value >>= 1;
            if ((this.P & CFlag) != 0)
            {
                value |= 0x80;
            }

            this.P &= 0x7c;

            if (value == 0)
            {
                this.P |= ZFlag;
            }

            if ((value & 0x80) != 0)
            {
                this.P |= NFlag;
            }

            this.P |= (byte)lowBit;

            this.bus.TickCpu();

            this.bus.TickCpu();
            this.bus.CpuWrite(this.address, value);
        }

        private void RorA()
        {
            var lowBit = this.A & 1;
            this.A >>= 1;
            if ((this.P & CFlag) != 0)
            {
                this.A |= 0x80;
            }

            this.P &= 0x7c;

            if (this.A == 0)
            {
                this.P |= ZFlag;
            }

            if ((this.A & 0x80) != 0)
            {
                this.P |= NFlag;
            }

            this.P |= (byte)lowBit;
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
            this.P |= CFlag;
        }

        private void Sei()
        {
            this.P |= 0x04;
        }

        private void Sbc()
        {
            var result = this.A - this.value;

            if ((this.P & CFlag) != 0)
            {
                result--;
            }

            this.A = (byte)(result & 0xff);

            this.P &= 0x7c;

            if ((this.A & 0x08) != 0)
            {
                this.P |= NFlag;
            }

            if (this.A == 0)
            {
                this.P |= ZFlag;
            }

            this.P |= (byte)(result >> 8);
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
            this.P &= 0x7d;

            if ((sbyte)a <= 0)
            {
                if (a == 0)
                {
                    this.P |= ZFlag;
                }
                else
                {
                    this.P |= NFlag;
                }
            }
        }

        private void SetCompareFlags(byte reg)
        {
            this.P &= 0x7c;

            if (reg < this.value)
            {
                this.P |= (byte)((reg & 0x80) >> 7);
            }
            else if (reg > this.value)
            {
                this.P |= CFlag;
                this.P |= (byte)((reg & 0x80) >> 7);
            }
            else
            {
                this.P |= ZFlag | CFlag;
            }
        }
    }
}
