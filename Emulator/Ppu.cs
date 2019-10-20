using System;

namespace NesEmu.Emulator
{
    public class Ppu
    {
        // approx!
        const int cyclesPerFrame = 29781;

        int cyclesThisFrame = 0;

        public int FrameCount = 0;

        private byte status;

        public byte[] Frame { get; } = new byte[256 * 240];

        public int currentScanLine = -1;
        public int scanlineCycle = 0;

        public void Tick()
        {
            this.DoTick();

            cyclesThisFrame++;
            if (cyclesThisFrame == cyclesPerFrame)
            {
                RenderFrame();
                cyclesThisFrame = 0;
            }
        }

        private void DoTick()
        {
            if (scanlineCycle == 1)
            {
                if (currentScanLine == 241)
                {
                    // The VBlank flag of the PPU is set at tick 1 (the second tick) of scanline 241
                    status |= 0x80;
                }
                else if (currentScanLine == -1)
                {
                    status &= 0x7f;
                }
            }

            scanlineCycle++;
            if (scanlineCycle == 341)
            {
                scanlineCycle = 0;

                currentScanLine++;
                if (currentScanLine == 261)
                {
                    currentScanLine = -1;
                }
            }
        }

        private unsafe void RenderFrame()
        {
            var random = new Random();
            var index = 0;

            fixed (byte* frameBytes = Frame)
            {
                for (var y = 0; y < 240; y++)
                {
                    for (var x = 0; x < 256; x++)
                    {
                        frameBytes[index++] = (byte)random.Next(0, 64);
                    }
                }
            }

            FrameCount++;

            this.OnFrame?.Invoke(this, EventArgs.Empty);
        }

        public byte Peek(ushort address)
        {
            address &= 0x07;

            if (address == 0x02)
            {
                return this.status;
            }

            return 0;
        }

        public byte Read(ushort address)
        {
            address &= 0x07;

            if (address == 0x02)
            {
                var status = this.status;
                this.status &= 0x7f;
                return status;
            }

            return 0;
        }


        public void Write(ushort address, byte value)
        {
            address &= 0x07;
        }

        public event EventHandler OnFrame;
    }
}
