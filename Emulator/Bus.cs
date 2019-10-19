using System;

namespace NesEmu.Emulator
{
    public class Bus
    {
        private Cart cart;
        private byte[] ram = new byte[2048];

        public byte Read(ushort address)
        {
            if (address < 0x2000)
            {
                return ram[address & 0x7ff];
            }

            byte value;
            if (cart.Read(address, out value))
            {
                return value;
            }

            return 0;
        }

        public void Write(ushort address, byte value)
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
