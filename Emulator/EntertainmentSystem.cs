using System;
using System.Threading;

namespace NesEmu.Emulator
{
    public class EntertainmentSystem
    {
        private NesCpu cpu;
        private Bus bus;

        private Thread emulationThread;
        private bool running;
        private bool stopRequested;

        public byte[] Frame { get; } = new byte[256 * 240];

        public EntertainmentSystem()
        {
            this.bus = new Bus();
            this.cpu = new NesCpu(this.bus);

            this.emulationThread = new Thread(this.Run);
        }

        public NesCpu Cpu => this.cpu;
        public Bus Bus => this.bus;

        public void InsertCart(Cart cart)
        {
            bus.Attach(cart);
        }

        public void Reset()
        {
            this.cpu.Reset();
        }

        public void Start()
        {
            if (this.running)
            {
                return;
            }

            this.running = true;
            this.emulationThread.Start();
        }

        public void Step()
        {
            if (this.running)
            {
                return;
            }

            this.Tick();

            this.Breaked?.Invoke(this, EventArgs.Empty);
        }

        public void Stop()
        {
            if (!this.running)
            {
                return;
            }

            this.stopRequested = true;
            this.emulationThread.Join();
            this.stopRequested = false;
            this.running = false;

            this.Breaked?.Invoke(this, EventArgs.Empty);
        }

        private void Run()
        {
            while (!this.stopRequested)
            {
                Tick();
                //Thread.Sleep(33);
            }
        }

        private unsafe void Tick()
        {
            this.Cpu.Tick();

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

            this.OnFrame?.Invoke(this, EventArgs.Empty);
        }

        public event EventHandler OnFrame;


        public void Break()
        {
            this.Breaked?.Invoke(this, EventArgs.Empty);
        }

        public event EventHandler Breaked;
    }
}
