using System;

namespace NesEmu.Emulator
{
    public class Ppu
    {
        // approx!
        const int cyclesPerFrame = 29781;

        int cyclesThisFrame = 0;

        public int FrameCount = 0;

        public byte[] Frame { get; } = new byte[256 * 240];

        public void Tick()
        {
            cyclesThisFrame++;
            if (cyclesThisFrame == cyclesPerFrame)
            {
                RenderFrame();
                cyclesThisFrame = 0;
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


        public event EventHandler OnFrame;
    }
}
