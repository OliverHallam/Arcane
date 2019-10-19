using NesEmu.Emulator;
using System;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Drawing.Imaging;
using System.Threading;
using System.Windows.Forms;

namespace NesEmu
{
    public partial class EmulatorForm : Form
    {
        private EntertainmentSystem gameSystem = new EntertainmentSystem();

        private Bitmap frame = new Bitmap(256, 240, PixelFormat.Format24bppRgb);

        public EmulatorForm()
        {
            this.Size = new Size(1024, 960);
            this.FormBorderStyle = FormBorderStyle.FixedSingle;
            this.DoubleBuffered = true;

            InitializeComponent();

            gameSystem.OnFrame += this.OnFrame;
            gameSystem.Start();
        }

        protected override void OnClosed(EventArgs e)
        {
            gameSystem.Stop();
        }

        protected unsafe void OnFrame(object sender, EventArgs e)
        {
            this.Invalidate();
        }

        private unsafe void CaptureFrame()
        {
            var data = frame.LockBits(new Rectangle(0, 0, 256, 240), ImageLockMode.WriteOnly, PixelFormat.Format24bppRgb);
            fixed (byte* framePtr = gameSystem.Frame)
            {
                var srcPtr = framePtr;
                var destPtr = (byte*)data.Scan0;

                for (var i = 0; i < 256 * 240; i++)
                {
                    *destPtr++ = *srcPtr;
                    *destPtr++ = *srcPtr;
                    *destPtr++ = *srcPtr;

                    srcPtr++;
                }
            }

            frame.UnlockBits(data);
        }

        protected override void OnDpiChanged(DpiChangedEventArgs e)
        {
            this.Invalidate();
            base.OnDpiChanged(e);
        }

        protected override void OnKeyDown(KeyEventArgs e)
        {
            Invalidate();
            base.OnKeyDown(e);
        }

        protected override void OnPaint(PaintEventArgs e)
        {
            base.OnPaint(e);

            this.CaptureFrame();

            var graphics = e.Graphics;
            graphics.InterpolationMode = InterpolationMode.NearestNeighbor;

            graphics.DrawImage(frame, 0, 0, this.Width, this.Height);
        }
    }
}
