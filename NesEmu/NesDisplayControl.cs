using System.Drawing;
using System.Windows.Forms;
using System.Drawing.Imaging;
using NesEmu.Emulator;
using System;
using System.Drawing.Drawing2D;

namespace NesEmu
{
    public partial class NesDisplayControl : Control
    {
        private Bitmap frame;
        private EntertainmentSystem system;

        public NesDisplayControl()
        {
            this.DoubleBuffered = true;

            this.frame = new Bitmap(256, 240, PixelFormat.Format8bppIndexed);
            NesColorPalette.SetPalette(frame.Palette);
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
                    this.system.Ppu.OnFrame -= OnFrame;
                }

                this.system = value;
                if (this.system != null)
                {
                    this.system.Ppu.OnFrame += OnFrame;
                }
            }
        }

        protected unsafe void OnFrame(object sender, EventArgs e)
        {
            this.Invalidate();
        }


        protected override void OnPaint(PaintEventArgs e)
        {
            base.OnPaint(e);

            var graphics = e.Graphics;

            if (this.system != null)
            {
                this.CaptureFrame();

                graphics.PixelOffsetMode = PixelOffsetMode.Half;
                graphics.InterpolationMode = InterpolationMode.NearestNeighbor;
                graphics.DrawImage(frame, 0, 0, this.Width, this.Height);
            }
            else
            {
                graphics.Clear(Color.Blue);
            }
        }

        private unsafe void CaptureFrame()
        {
            var data = frame.LockBits(new Rectangle(0, 0, 256, 240), ImageLockMode.WriteOnly, PixelFormat.Format8bppIndexed);
            fixed (byte* framePtr = this.system.Ppu.Frame)
            {
                var srcPtr = framePtr;
                var destPtr = (byte*)data.Scan0;

                for (var i = 0; i < 256 * 240; i++)
                {
                    *destPtr++ = *srcPtr++;
                }
            }

            frame.UnlockBits(data);
        }
    }
}
