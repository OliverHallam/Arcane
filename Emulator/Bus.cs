using System;

namespace NesEmu.Emulator
{
    public class Bus
    {
        private NesCpu cpu;
        private Ppu ppu;

        private Controller controller;
        private Cart cart;

        private byte[] cpuRam = new byte[2048];
        private byte[] ppuRam = new byte[2048];

        public ushort LastWriteAddress { get; set; }


        public Bus()
        {
            for (var i=0; i<2048; i++)
            {
                cpuRam[i] = 0xff;
                ppuRam[i] = 0xff;
            }
        }

        public byte Peek(ushort address)
        {
            if (address < 0x2000)
            {
                return cpuRam[address & 0x7ff];
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
                return this.cpuRam[address & 0x7ff];
            }

            if (address < 0x4000)
            {
                return this.ppu.Read(address);
            }

            if (address < 0x4020)
            {
                if (address == 0x4016)
                {
                    return this.controller.Read();
                }
                return 0;
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
                cpuRam[address & 0x7ff] = value;
            }
            else if (address < 0x4000)
            {
                ppu.Write(address, value);
            }
            else if (address < 0x4020)
            {
                if (address == 0x4016)
                {
                    this.controller.Write(value);
                }
            }
        }

        internal void SignalNmi()
        {
            this.cpu.SignalNmi();
        }

        public byte PpuRead(ushort address)
        {
            if (address < 0x2000)
            {
                return cart.PpuRead(address);
            }

            // TODO: more general nametable mirroring
            address &= 0xfbff;

            return this.ppuRam[address & 0x07ff];
        }

        public void PpuWrite(ushort address, byte value)
        {
            if (address >= 0x2000)
            {
                // TODO: more general nametable mirroring
                address &= 0xfbff;

                this.ppuRam[address & 0x07ff] = value;
            }
        }

        public void TickCpu()
        {
            this.ppu.Tick();
        }

        public void Attach(Cart cart)
        {
            this.cart = cart;
        }

        internal void Attach(NesCpu cpu)
        {
            this.cpu = cpu;
        }

        internal void Attach(Ppu ppu)
        {
            this.ppu = ppu;
        }

        internal void Attach(Controller controller)
        {
            this.controller = controller;
        }
    }
}
