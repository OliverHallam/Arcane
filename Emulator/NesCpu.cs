using System;
using System.Collections.Generic;
using System.Text;

namespace NesEmu.Emulator
{
    public class NesCpu
    {
        private const byte CFlag = 0x01;
        private const byte ZFlag = 0x02;
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

        public void RunInstruction()
        {
            var opCode = bus.CpuRead(this.PC++);
            this.bus.TickCpu();

            switch(opCode)
            {
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

                case 0x29:
                    this.Immediate();
                    this.And();
                    return;

                case 0x45:
                    this.ZeroPage();
                    this.Load();
                    this.Eor();
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

                case 0x88:
                    this.bus.TickCpu();
                    this.Dey();
                    return;

                case 0x8a:
                    this.bus.TickCpu();
                    this.Txa();
                    return;

                case 0x8d:
                    this.Absolute();
                    this.Sta();
                    return;

                case 0x91:
                    this.IndirectIndexed();
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

                case 0xa0:
                    this.Immediate();
                    this.Ldy();
                    break;

                case 0xa2:
                    this.Immediate();
                    this.Ldx();
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

                case 0xad:
                    this.Absolute();
                    this.Load();
                    this.Lda();
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

                case 0xd0:
                    this.Relative();
                    this.Bne();
                    return;

                case 0xd8:
                    this.bus.TickCpu();
                    this.Cld();
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

        private void ZeroPage()
        {
            this.address = bus.CpuRead(this.PC++);
            this.bus.TickCpu();
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

        private void Beq()
        {
            if ((this.P & ZFlag) != 0)
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
            this.P &= 0x7c;

            if (this.A < value)
            {
                this.P |= (byte)((this.A & 0x80) >> 7);
            }
            else if (this.A > value)
            {
                this.P |= CFlag;
                this.P |= (byte)((this.A & 0x80) >> 7);
            }
            else
            {
                this.P |= ZFlag | CFlag;
            }
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

        private void Sei()
        {
            this.P |= 0x04;
        }

        private void Sta()
        {
            this.bus.TickCpu();
            this.bus.CpuWrite(this.address, this.A);
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

    }
}
