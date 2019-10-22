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


        protected override void OnClosed(EventArgs e)
        {
            gameSystem.Stop();
        }

        protected override void OnDpiChanged(DpiChangedEventArgs e)
        {
            this.Invalidate();
            base.OnDpiChanged(e);
        }

        protected override bool ProcessCmdKey(ref Message msg, Keys keyData)
        {
            switch (keyData)
            {
                case Keys.Up:
                    gameSystem.Controller.Up = true;
                    return true;

                case Keys.Down:
                    gameSystem.Controller.Down = true;
                    return true;

                case Keys.Left:
                    gameSystem.Controller.Left = true;
                    return true;

                case Keys.Right:
                    gameSystem.Controller.Right = true;
                    return true;
            }

            return false;
        }

        protected override void OnKeyDown(KeyEventArgs e)
        {
            switch (e.KeyCode)
            {
                case Keys.Space:
                    gameSystem.Step();
                    break;

                case Keys.F5:
                    this.dissassembly.ProgramCounter = 0;
                    gameSystem.Start();
                    break;

                case Keys.F8:
                    gameSystem.RunFrame();
                    break;

                case Keys.P:
                    gameSystem.Stop();
                    break;

                case Keys.Z:
                    gameSystem.Controller.A = true;
                    break;
                case Keys.X:
                    gameSystem.Controller.B = true;
                    break;
                case Keys.Q:
                    gameSystem.Controller.Select = true;
                    break;
                case Keys.W:
                    gameSystem.Controller.Start = true;
                    break;
            }

            base.OnKeyDown(e);
        }

        protected override void OnKeyUp(KeyEventArgs e)
        {
            switch (e.KeyCode)
            {
                case Keys.Up:
                    gameSystem.Controller.Up = false;
                    break;
                case Keys.Down:
                    gameSystem.Controller.Down = false;
                    break;
                case Keys.Left:
                    gameSystem.Controller.Left = false;
                    break;
                case Keys.Right:
                    gameSystem.Controller.Right = false;
                    break;

                case Keys.Z:
                    gameSystem.Controller.A = false;
                    break;
                case Keys.X:
                    gameSystem.Controller.B = false;
                    break;
                case Keys.Q:
                    gameSystem.Controller.Select = false;
                    break;
                case Keys.W:
                    gameSystem.Controller.Start = false;
                    break;
            }

            base.OnKeyUp(e);
        }
    }
}
