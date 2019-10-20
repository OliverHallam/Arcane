using NesEmu.Emulator;
using System;
using System.Drawing;
using System.Windows.Forms;

namespace NesEmu
{
    public partial class EmulatorForm : Form
    {
        private EntertainmentSystem gameSystem;

        public EmulatorForm()
        {
            InitializeComponent();

            this.KeyPreview = true;
        }

        public EntertainmentSystem EntertainmentSystem
        {
            get
            {
                return this.gameSystem;
            }

            set
            {
                this.gameSystem = value;
                this.nesDisplay.System = value;
                this.registers.Cpu = value.Cpu;
                this.dissassembly.Bus = value.Bus;

                value.Cpu.Ticked += OnCpuTick;
            }
        }

        private void OnCpuTick(object sender, EventArgs e)
        {
            this.dissassembly.StartAddress = this.dissassembly.ProgramCounter = ((NesCpu)sender).PC;
        }

        protected override void OnKeyDown(KeyEventArgs e)
        {
            if (e.KeyCode == Keys.Space)
            {
                gameSystem.Step();
            }

            if (e.KeyCode == Keys.F5)
            {
                gameSystem.Start();
            }
        }

        protected override void OnClosed(EventArgs e)
        {
            gameSystem.Stop();
        }

        protected override void OnDpiChanged(DpiChangedEventArgs e)
        {
            this.Invalidate();
            base.OnDpiChanged(e);
        }
    }
}
