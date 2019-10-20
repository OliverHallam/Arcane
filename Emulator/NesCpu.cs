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

            this.Ticked?.Invoke(this, EventArgs.Empty);
        }

        public event EventHandler Ticked;

        private void RunInstruction()
        {
            var opCode = bus.Read(this.PC++);
            CycleCount++;

            switch(opCode)
            {
                case 0x29:
                    Immediate();
                    And();
                    return;

                case 0x78:
                    CycleCount++;
                    Sei();
                    return;

                case 0x8d:
                    Absolute();
                    Sta();
                    return;

                case 0x9a:
                    CycleCount++;
                    Txs();
                    return;

                case 0xa2:
                    Immediate();
                    Ldx();
                    return;

                case 0xa9:
                    Immediate();
                    Lda();
                    return;

                case 0xad:
                    Absolute();
                    Load();
                    Lda();
                    return;

                case 0xd8:
                    CycleCount++;
                    Cld();
                    return;

                case 0xf0:
                    Relative();
                    Beq();
                    return;
            }
        }

        private void Immediate()
        {
            this.value = bus.Read(this.PC++);
            CycleCount++;
        }

        private void Absolute()
        {
            this.address = (ushort)(this.bus.Read(this.PC++) << 8 | this.bus.Read(this.PC++));
            CycleCount += 2;
        }

        private void Relative()
        {
            var relative = (sbyte)(this.bus.Read(this.PC++));
            this.address = (ushort)(this.PC + relative);
            CycleCount++;
        }

        private void Load()
        {
            this.value = bus.Read(this.address);
            CycleCount++;
        }

        private void And()
        {
            this.A &= this.value;
            SetFlags(this.A);
        }

        private void Beq()
        {
            if ((this.P & ZFlag) != 0)
            {
                Jump();
            }
        }

        private void Cld()
        {
            this.P &= 0xf7;
        }

        private void Lda()
        {
            this.A = value;
            SetFlags(this.A);
        }

        private void Ldx()
        {
            this.X = value;
            SetFlags(this.X);
        }

        private void Sei()
        {
            this.P |= 0x04;
        }

        private void Sta()
        {
            this.bus.Write(this.address, this.A);
            CycleCount++;
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
                CycleCount++;
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
