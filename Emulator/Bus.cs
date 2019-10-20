using System;

namespace NesEmu.Emulator
{
    public class Bus
    {
        private Cart cart;
        private byte[] ram = new byte[2048];

        public byte CpuRead(ushort address)
        {
            if (address < 0x2000)
            {
                return ram[address & 0x7ff];
            }

            if (address == 0x2002)
            {
                // hack to get games booting
                return 0x80;
            }


            byte value;
            if (cart != null && cart.CpuRead(address, out value))
            {
                return value;
            }

            return 0;
        }

        public byte PpuRead(ushort address)
        {
            return cart.PpuRead(address);
        }

        public void CpuWrite(ushort address, byte value)
        {
            if (address < 0x2000)
            {
                ram[address & 0x7ff] = value;
            }
        }

        internal void Attach(Cart cart)
        {
            this.cart = cart;
        }
    }
}
