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

            this.patternLeft.TableAddress = 0;
            this.patternRight.TableAddress = 0x1000;
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

                this.patternLeft.System = value;
                this.patternRight.System = value;

                this.memory.Bus = value.Bus;

                value.Breaked += OnBreak;
            }
        }

        private void OnBreak(object sender, EventArgs e)
        {
            this.dissassembly.ProgramCounter = this.gameSystem.Cpu.PC;

            if (this.dissassembly.ProgramCounter < this.dissassembly.StartAddress ||
                this.dissassembly.ProgramCounter >= this.dissassembly.StartAddress + this.dissassembly.InstructionCount)
            {
                this.dissassembly.StartAddress = this.dissassembly.ProgramCounter;
            }

            this.memory.LastAcccessAddress = this.gameSystem.Bus.LastWriteAddress;
            this.memory.BaseAddress = (ushort)(this.gameSystem.Bus.LastWriteAddress & 0xfe00);
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

            if (e.KeyCode == Keys.F)
            {
                gameSystem.RunFrame();
            }

            if (e.KeyCode == Keys.P)
            {
                gameSystem.Stop();
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
