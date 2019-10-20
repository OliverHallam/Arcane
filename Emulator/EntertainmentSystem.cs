using System;
using System.Diagnostics;
using System.Threading;

namespace NesEmu.Emulator
{
    public class EntertainmentSystem
    {
        private Ppu ppu;
        private NesCpu cpu;
        private Bus bus;

        private Thread emulationThread;
        private bool running;
        private bool stopRequested;

        private ushort breakpoint;

        public EntertainmentSystem()
        {
            this.bus = new Bus();
            this.ppu = new Ppu(this.bus);
            this.cpu = new NesCpu(this.bus);

            bus.Attach(cpu);
            bus.Attach(ppu);
        }

        public NesCpu Cpu => this.cpu;
        public Ppu Ppu => this.ppu;
        public Bus Bus => this.bus;

        public void InsertCart(Cart cart)
        {
            bus.Attach(cart);
        }

        public void Reset()
        {
            this.PoweredUp?.Invoke(this, EventArgs.Empty);

            this.cpu.Reset();
        }

        public void Start()
        {
            if (this.running)
            {
                return;
            }

            this.running = true;

            this.emulationThread = new Thread(this.Run);
            this.emulationThread.Start();
        }

        public void Step()
        {
            if (this.running)
            {
                return;
            }

            this.Cpu.RunInstruction();

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
            var frameInTicks = TimeSpan.TicksPerSecond / 60;
            var stopwatch = Stopwatch.StartNew();

            while (!this.stopRequested)
            {
                stopwatch.Reset();
                stopwatch.Start();

                int currentFrame = ppu.FrameCount;
                while (ppu.FrameCount == currentFrame)
                {
                    if (this.stopRequested)
                    {
                        return;
                    }

                    if (this.Cpu.PC == breakpoint)
                    {
                        this.stopRequested = false;
                        this.running = false;

                        this.Breaked?.Invoke(this, EventArgs.Empty);
                        return;
                    }

                    this.Cpu.RunInstruction();
                }

                SpinWait.SpinUntil(() => stopwatch.ElapsedTicks >= frameInTicks);
            }
        }

        public void RunFrame()
        {
            if (this.running)
            {
                return;
            }

            int currentFrame = ppu.FrameCount;
            while (ppu.FrameCount == currentFrame)
            {
                this.Cpu.RunInstruction();
            }

            this.Breaked?.Invoke(this, EventArgs.Empty);
        }


        public void Break()
        {
            this.Breaked?.Invoke(this, EventArgs.Empty);
        }

        public event EventHandler Breaked;

        public event EventHandler PoweredUp;
    }
}
