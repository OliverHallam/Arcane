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

        public int CycleCount;

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
            this.PC = (ushort)(bus.Read(0xfffd) << 8 | bus.Read(0xfffc));
        }

        public void Tick()
        {
            this.RunInstruction();
        }

        private void RunInstruction()
        {
            var opCode = bus.Read(this.PC++);
            this.CycleCount++;

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
                    this.CycleCount++;
                    this.Sei();
                    return;

                case 0x84:
                    this.ZeroPage();
                    this.Sty();
                    return;

                case 0x88:
                    this.CycleCount++;
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
                    this.CycleCount++;
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
                    this.CycleCount++;
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
            this.value = bus.Read(this.PC++);
            this.CycleCount++;
        }

        private void Absolute()
        {
            this.address = (ushort)(this.bus.Read(this.PC++) | this.bus.Read(this.PC++) << 8);
            this.CycleCount += 2;
        }

        private void ZeroPage()
        {
            this.address = bus.Read(this.PC++);
            this.CycleCount++;
        }

        private void Relative()
        {
            var relative = (sbyte)(this.bus.Read(this.PC++));
            this.address = (ushort)(this.PC + relative);
            this.CycleCount++;
        }

        private void IndirectIndexed()
        {
            ushort indirectAddress = bus.Read(this.PC++);
            this.address = (ushort)(this.bus.Read(indirectAddress) | this.bus.Read((ushort)(indirectAddress + 1)) << 8);
            this.address += this.Y;
            this.CycleCount += 4;
        }

        private void Load()
        {
            this.value = bus.Read(this.address);
            this.CycleCount++;
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
            var value = this.bus.Read(this.address);
            value--;
            this.bus.Write(this.address, value);
            this.CycleCount += 3;
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
            this.bus.Write(this.address, this.A);
            this.CycleCount++;
        }

        private void Sty()
        {
            this.bus.Write(this.address, this.Y);
            this.CycleCount++;
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
                this.CycleCount++;
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
