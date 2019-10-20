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
                this.registers.System = value;
                this.dissassembly.Bus = value.Bus;

                value.Breaked += OnBreak;
            }
        }

        private void OnBreak(object sender, EventArgs e)
        {
            this.dissassembly.ProgramCounter = this.gameSystem.Cpu.PC;

            if (this.dissassembly.ProgramCounter >= this.dissassembly.StartAddress + this.dissassembly.InstructionCount)
            {
                this.dissassembly.StartAddress = this.dissassembly.ProgramCounter;
            }
        }

        protected override void OnKeyDown(KeyEventArgs e)
        {
            if (e.KeyCode == Keys.Space)
            {
                gameSystem.Step();
            }

            if (e.KeyCode == Keys.F5)
            {
                this.dissassembly.ProgramCounter = 0;
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
