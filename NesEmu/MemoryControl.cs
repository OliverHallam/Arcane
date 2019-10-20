using NesEmu.Emulator;
using System;
using System.Drawing;
using System.Windows.Forms;

namespace NesEmu
{
    class MemoryControl : Control
    {
        private Bus bus;

        private SolidBrush addressBrush;
        private Pen seperatorPen;
        private SolidBrush valuesBrush;
        private SolidBrush lastAccessedBrush;

        private ushort baseAddress;
        private ushort lastAccessAddress = 0xffff;

        private int padding;
        private int splitX;
        private float valuesX;
        private int lineHeight;
        private float byteWidth;

        public MemoryControl()
        {
            this.DoubleBuffered = true;
            this.addressBrush = new SolidBrush(Color.DimGray);
            this.seperatorPen = new Pen(new SolidBrush(Color.DimGray));
            this.valuesBrush = new SolidBrush(Color.Gray);
            this.lastAccessedBrush = new SolidBrush(Color.LightGray);
        }

        protected override void Dispose(bool disposing)
        {
            if (disposing)
            {
                this.addressBrush.Dispose();
                this.seperatorPen.Dispose();
                this.valuesBrush.Dispose();
                this.lastAccessedBrush.Dispose();
            }

            base.Dispose(disposing);
        }


        public Bus Bus
        {
            get
            {
                return this.bus;
            }
            set
            {
                this.bus = value;
                Invalidate();
            }
        }

        public ushort BaseAddress
        {
            get
            {
                return this.baseAddress;
            }
            set
            {
                this.baseAddress = value;
                Invalidate();
            }
        }

        public ushort LastAcccessAddress
        {
            get
            {
                return this.lastAccessAddress;
            }
            set
            {
                this.lastAccessAddress = value;
                Invalidate();
            }
        }

        protected override void OnFontChanged(EventArgs e)
        {
            this.padding = this.Font.Height / 2;

            var linePadding = this.Font.Height / 8;
            this.lineHeight = this.Font.Height + linePadding;

            base.OnFontChanged(e);
        }

        protected override void OnPaint(PaintEventArgs e)
        {
            if (this.bus == null)
            {
                return;
            }

            var graphics = e.Graphics;

            var addressSize = graphics.MeasureString("0x0000", this.Font).Width;
            this.splitX = (int)addressSize + padding * 2;
            this.valuesX = splitX + padding;
            this.byteWidth = graphics.MeasureString("00 ", this.Font).Width;

            var y = padding;
            for (ushort currentAddress = this.BaseAddress; currentAddress < this.BaseAddress + 512; currentAddress += 16)
            {
                this.DrawMemory(graphics, y, currentAddress);
                y += lineHeight;
            }

            graphics.DrawLine(this.seperatorPen, this.splitX, padding, this.splitX, y - lineHeight + this.Font.Height);

            base.OnPaint(e);
        }

        private void DrawMemory(Graphics graphics, int y, ushort currentAddress)
        {
            graphics.DrawString("0x" + currentAddress.ToString("X4"), this.Font, this.addressBrush, padding, y);

            var x = this.valuesX;
            for (var i = 0; i < 16; i++)
            {
                var memory = this.bus.CpuRead(currentAddress);
                graphics.DrawString(memory.ToString("X2"), this.Font, currentAddress == lastAccessAddress ? this.lastAccessedBrush : this.valuesBrush, x, y);
                x += this.byteWidth;

                currentAddress++;
            }
        }
    }
}
