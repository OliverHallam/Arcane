using NesEmu.Emulator;
using System;
using System.Drawing;
using System.Windows.Forms;

namespace NesEmu
{
    public partial class EmulatorForm : Form
    {
        private EntertainmentSystem gameSystem = new EntertainmentSystem();

        public EmulatorForm()
        {
            InitializeComponent();

            this.KeyPreview = true;

            this.nesDisplay.System = gameSystem;
            this.registers.Cpu = gameSystem.Cpu;
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
