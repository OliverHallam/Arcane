using System;
using System.Collections.Generic;
using System.Text;

namespace NesEmu.Emulator
{
    public class Controller
    {
        public bool Up;
        public bool Down;
        public bool Left;
        public bool Right;

        public bool A;
        public bool B;

        public bool Select;
        public bool Start;

        private bool strobe;
        private byte noise;
        private byte state;

        public void Write(byte data)
        {
            if ((data & 0x01) == 0 && this.strobe)
            {
                CaptureState();
            }

            this.noise = (byte)(data & 0xf4);
            this.strobe = (data & 0x01) != 0;
        }

        public byte Read()
        {
            if (this.strobe)
            {
                this.CaptureState();
            }

            var status = this.state >> 7;
            this.state <<= 1;
            this.state |= 1;

            return (byte)(this.noise | status);
        }


        private void CaptureState()
        {
            this.state = 0;
            if (this.A)
                this.state |= 0x80;
            if (this.B)
                this.state |= 0x40;
            if (this.Select)
                this.state |= 0x20;
            if (this.Start)
                this.state |= 0x10;
            if (this.Up)
                this.state |= 0x08;
            if (this.Down)
                this.state |= 0x04;
            if (this.Left)
                this.state |= 0x02;
            if (this.Right)
                this.state |= 0x01;
        }
    }
}
