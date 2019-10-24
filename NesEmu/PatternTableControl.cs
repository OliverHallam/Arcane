using NesEmu.Emulator;
using System;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Drawing.Imaging;
using System.Windows.Forms;

namespace NesEmu
{
    class PatternTableControl : Control
    {
        private Bitmap frame;
        private EntertainmentSystem system;

        private ushort tableAddress;

        public PatternTableControl()
        {
            this.DoubleBuffered = true;

            this.frame = new Bitmap(128, 128, PixelFormat.Format8bppIndexed);

            var palette = frame.Palette;
            NesColorPalette.UpdatePalette(palette);
            frame.Palette = palette;
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
                    this.system.Ppu.OnFrame -= this.OnFrame;
                }

                this.system = value;

                if (this.system != null)
                {
                    this.system.Ppu.OnFrame += this.OnFrame;
                }
            }
        }

        public ushort TableAddress
        {
            get
            {
                return tableAddress;
            }
            set
            {
                this.tableAddress = value;
                this.Invalidate();
            }
        }

        protected override void OnPaint(PaintEventArgs e)
        {
            base.OnPaint(e);

            var graphics = e.Graphics;

            if (this.system != null)
            {
                this.CapturePatterns();

                graphics.PixelOffsetMode = PixelOffsetMode.Half;
                graphics.InterpolationMode = InterpolationMode.NearestNeighbor;
                graphics.DrawImage(frame, 0, 0, this.Width, this.Height);
            }
            else
            {
                graphics.Clear(Color.Blue);
            }
        }

        private void OnFrame(object sender, EventArgs e)
        {
            this.Invalidate();
        }

        private unsafe void CapturePatterns()
        {
            var data = frame.LockBits(new Rectangle(0, 0, 128, 128), ImageLockMode.WriteOnly, PixelFormat.Format8bppIndexed);

            var destPtr = (byte*)data.Scan0;

            for (var tileY = 0; tileY < 16; tileY++)
            {
                for (var y = 0; y < 8; y++)
                {
                    for (var tileX = 0; tileX < 16; tileX++)
                    {
                        var tileAddress = (ushort)(this.TableAddress | (tileY << 8) | (tileX << 4) | y);
                        var tileByteLow = this.system.Bus.PpuRead(tileAddress);
                        var tileByteHigh = this.system.Bus.PpuRead((ushort)(tileAddress | 8));

                        for (var x = 0; x < 8; x++)
                        {
                            var index = (byte)((tileByteHigh & 0x80) >> 6 | (tileByteLow & 0x80) >> 7);
                            tileByteHigh <<= 1;
                            tileByteLow <<= 1;

                            *destPtr++ = this.system.Ppu.Palette[index];
                        }
                    }
                }
            }

            frame.UnlockBits(data);
        }
    }
}
