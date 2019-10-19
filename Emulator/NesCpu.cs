using System;
using System.Collections.Generic;
using System.Text;

namespace NesEmu.Emulator
{
    public class NesCpu
    {
        // Registers
        public byte A;
        public byte X;
        public byte Y;
        public ushort PC;
        public byte S;
        public byte P;

        public int CycleCount;

        public void Tick()
        {
            CycleCount++;

            A++;

            this.Ticked?.Invoke(this, EventArgs.Empty);
        }

        public event EventHandler Ticked;
    }
}
