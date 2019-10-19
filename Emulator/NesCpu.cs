using System;
using System.Collections.Generic;
using System.Text;

namespace NesEmu.Emulator
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
        public byte P;

        public int CycleCount;

        public NesCpu(Bus bus)
        {
            this.bus = bus;
        }

        public void Reset()
        {
            // 7 cycles

            this.PC = (ushort)(bus.Read(0xfffc) << 8 | bus.Read(0xfffd));
        }

        public void Tick()
        {


            CycleCount++;
            this.Ticked?.Invoke(this, EventArgs.Empty);
        }

        public event EventHandler Ticked;
    }
}
