using System;
using System.Collections.Generic;
using System.Text;

namespace NesEmu.Emulator
{
    public class NesCpu
    {
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

                case 0x29:
                    this.Immediate();
                    this.And();
                    return;

                case 0x78:
                    this.bus.TickCpu();
                    this.Sei();
                    return;

                case 0x84:
                    this.ZeroPage();
                    this.Sty();
                    return;

                case 0x88:
                    this.bus.TickCpu();
                    this.Dey();
                    return;

                case 0x8d:
                    this.Absolute();
                    this.Sta();
                    return;

                case 0x91:
                    this.IndirectIndexed();
                    this.Sta();
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

                case 0xa9:
                    this.Immediate();
                    this.Lda();
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
            }
        }

        private void Immediate()
        {
            this.value = bus.CpuRead(this.PC++);
            this.bus.TickCpu();
        }

        private void Absolute()
        {
            this.address = (ushort)(this.bus.CpuRead(this.PC++) | this.bus.CpuRead(this.PC++) << 8);
            this.bus.TickCpu();
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

        private void Cld()
        {
            this.P &= 0xf7;
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

        private void Dey()
        {
            this.Y--;
            this.SetFlags(this.Y);
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

        private void Txs()
        {
            this.S = this.X;
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
