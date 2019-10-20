using System;

namespace NesEmu.Emulator
{
    public class Bus
    {
        private Ppu ppu;

        private Cart cart;
        private byte[] ram = new byte[2048];

        public ushort LastWriteAddress { get; set; }


        public Bus(Ppu ppu)
        {
            for (var i=0; i<2048; i++)
            {
                ram[i] = 0xff;
            }

            this.ppu = ppu;
        }

        public byte Peek(ushort address)
        {
            if (address < 0x2000)
            {
                return ram[address & 0x7ff];
            }

            if (address < 0x4000)
            {
                ppu.Peek(address);
            }

            byte value;
            if (cart != null && cart.CpuRead(address, out value))
            {
                return value;
            }

            return 0;
        }

        public byte CpuRead(ushort address)
        {
            if (address < 0x2000)
            {
                return ram[address & 0x7ff];
            }

            if (address < 0x4000)
            {
                return ppu.Read(address);
            }

            byte value;
            if (cart != null && cart.CpuRead(address, out value))
            {
                return value;
            }

            return 0;
        }

        public void CpuWrite(ushort address, byte value)
        {
            this.LastWriteAddress = address;

            if (address < 0x2000)
            {
                ram[address & 0x7ff] = value;
            }
        }

        public byte PpuRead(ushort address)
        {
            return cart.PpuRead(address);
        }

        public void TickCpu()
        {
            this.ppu.Tick();
        }

        internal void Attach(Cart cart)
        {
            this.cart = cart;
        }
    }
}
