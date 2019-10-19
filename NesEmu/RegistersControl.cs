using NesEmu.Emulator;
using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace WindowsFormsApp1
{
    class RegistersControl : Control
    {
        private NesCpu cpu;

        private int a;
        private int x;
        private int y;
        private int pc;
        private int s;
        private int p;

        private SolidBrush nameBrush;
        private Pen seperatorPen;
        private SolidBrush valueBrush;
        private int padding;
        private int lineHeight;
        private float nameRight;
        private int splitX;
        private float valueX;

        public RegistersControl()
        {
            this.DoubleBuffered = true;
            this.nameBrush = new SolidBrush(Color.Gray);
            this.valueBrush = new SolidBrush(Color.LightGray);
            this.seperatorPen = new Pen(new SolidBrush(Color.DimGray));
        }

        protected override void Dispose(bool disposing)
        {
            if (disposing)
            {
                this.nameBrush.Dispose();
                this.seperatorPen.Dispose();
            }

            base.Dispose(disposing);
        }

        public NesCpu Cpu
        {
            get
            {
                return this.cpu;
            }

            set
            {
                if (this.cpu != null)
                {
                    this.cpu.Ticked -= OnTick;
                }

                this.cpu = value;

                if (this.cpu != null)
                {
                    this.cpu.Ticked += OnTick;
                }

                Invalidate();
            }
        }

        protected override void OnFontChanged(EventArgs e)
        {
            this.padding = Font.Height / 2;

            var linePadding = Font.Height / 4;
            this.lineHeight = Font.Height + linePadding;

            base.OnFontChanged(e);
        }

        protected override void OnPaint(PaintEventArgs e)
        {
            base.OnPaint(e);

            var graphics = e.Graphics;

            var pcSize = graphics.MeasureString("PC", this.Font);
            this.nameRight = pcSize.Width + padding;
            this.splitX = (int)nameRight + padding;
            this.valueX = splitX + padding;

            var y = padding;
            this.DrawRegister(graphics, y, "A", this.a.ToString("X2"));

            y += lineHeight;
            this.DrawRegister(graphics, y, "X", this.x.ToString("X2"));

            y += lineHeight;
            this.DrawRegister(graphics, y, "Y", this.y.ToString("X2"));

            y += lineHeight;
            this.DrawRegister(graphics, y, "PC", this.pc.ToString("X4"));

            y += lineHeight;
            this.DrawRegister(graphics, y, "S", this.s.ToString("X2"));

            y += lineHeight;
            this.DrawRegister(graphics, y, "P", this.p.ToString("X2"));

            graphics.DrawLine(this.seperatorPen, this.splitX, padding, this.splitX, y + this.Font.Height);
        }

        private void OnTick(object sender, EventArgs e)
        {
            this.a = this.cpu.A;
            this.x = this.cpu.X;
            this.y = this.cpu.Y;
            this.pc = this.cpu.PC;
            this.s = this.cpu.S;
            this.p = this.cpu.P;

            this.Invalidate();
        }

        private void DrawRegister(Graphics graphics, int y, string registerName, string value)
        {
            var size = graphics.MeasureString(registerName, this.Font);
            graphics.DrawString(registerName, this.Font, this.nameBrush, nameRight - size.Width, y);
            graphics.DrawString(value, this.Font, this.valueBrush, this.valueX, y);
        }
    }
}
