using NesEmu.Emulator;
using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace NesEmu
{
    class RegistersControl : Control
    {
        private EntertainmentSystem system;

        private int a;
        private int x;
        private int y;
        private int pc;
        private int s;
        private int p;

        private SolidBrush nameBrush;
        private Pen seperatorPen;
        private SolidBrush valueBrush;
        private SolidBrush disabledBrush;
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
            this.disabledBrush = new SolidBrush(Color.DimGray);
            this.seperatorPen = new Pen(new SolidBrush(Color.DimGray));
        }

        protected override void Dispose(bool disposing)
        {
            if (disposing)
            {
                this.nameBrush.Dispose();
                this.seperatorPen.Dispose();
                this.valueBrush.Dispose();
                this.disabledBrush.Dispose();
            }

            base.Dispose(disposing);
        }

        public EntertainmentSystem System
        {
            get
            {
                return this.system;
            }

            set
            {
                if (this.system != null)
                {
                    this.system.Breaked -= OnTick;
                }

                this.system = value;

                if (this.system != null)
                {
                    this.system.Breaked += OnTick;
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
            this.DrawFlags(graphics, y, "P", (byte)this.p);

            graphics.DrawLine(this.seperatorPen, this.splitX, padding, this.splitX, y + this.Font.Height);
        }

        private void OnTick(object sender, EventArgs e)
        {
            var cpu = this.system.Cpu;
            this.a = cpu.A;
            this.x = cpu.X;
            this.y = cpu.Y;
            this.pc = cpu.PC;
            this.s = cpu.S;
            this.p = cpu.P;

            this.Invalidate();
        }

        private void DrawRegister(Graphics graphics, int y, string registerName, string value)
        {
            var size = graphics.MeasureString(registerName, this.Font);
            graphics.DrawString(registerName, this.Font, this.nameBrush, nameRight - size.Width, y);
            graphics.DrawString(value, this.Font, this.valueBrush, this.valueX, y);
        }

        private void DrawFlags(Graphics graphics, int y, string registerName, byte value)
        {
            var size = graphics.MeasureString(registerName, this.Font);
            graphics.DrawString(registerName, this.Font, this.nameBrush, nameRight - size.Width, y);

            var x = valueX;

            graphics.DrawString("C", this.Font, (value & 0x01) != 0 ? this.valueBrush : this.disabledBrush, x, y);
            x += graphics.MeasureString("C ", this.Font).Width;

            graphics.DrawString("Z", this.Font, (value & 0x02) != 0 ? this.valueBrush : this.disabledBrush, x, y);
            x += graphics.MeasureString("Z ", this.Font).Width;

            graphics.DrawString("I", this.Font, (value & 0x04) != 0 ? this.valueBrush : this.disabledBrush, x, y);
            x += graphics.MeasureString("I ", this.Font).Width;

            graphics.DrawString("D", this.Font, (value & 0x08) != 0 ? this.valueBrush : this.disabledBrush, x, y);
            x += graphics.MeasureString("D ", this.Font).Width;

            graphics.DrawString("V", this.Font, (value & 0x40) != 0 ? this.valueBrush : this.disabledBrush, x, y);
            x += graphics.MeasureString("V ", this.Font).Width;

            graphics.DrawString("N", this.Font, (value & 0x80) != 0 ? this.valueBrush : this.disabledBrush, x, y);
        }
    }
}
