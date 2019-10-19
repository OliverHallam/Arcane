using System;
using System.Threading;

namespace NesEmu.Emulator
{
    public class EntertainmentSystem
    {
        private Thread emulationThread;
        private bool stopRequested;

        public byte[] Frame { get; } = new byte[256 * 240];

        public EntertainmentSystem()
        {
            this.emulationThread = new Thread(this.Run);
        }

        private void Tick()
        {
            var random = new Random();
            var frameBytes = new Span<byte>(this.Frame);
            var index = 0;
            for (var y=0; y<240; y++)
            {
                for (var x = 0; x<256; x++)
                {
                    frameBytes[index++] = (byte)random.Next(0, 256);
                }
            }

            this.OnFrame?.Invoke(this, EventArgs.Empty);
        }

        public void Start()
        {
            this.emulationThread.Start();
        }

        public void Stop()
        {
            this.stopRequested = true;
            this.emulationThread.Join();
            this.stopRequested = false;
        }


        private void Run()
        {
            while (!this.stopRequested)
            {
                Tick();
                //Thread.Sleep(33);
            }
        }



        public event EventHandler OnFrame;
    }
}
