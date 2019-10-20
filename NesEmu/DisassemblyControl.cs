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
    public class DisassemblyControl : Control
    {
        private Bus bus;

        private ushort startAddress;
        private ushort programCounter;
        private int instructionCount;

        private SolidBrush addressBrush;
        private Pen seperatorPen;
        private SolidBrush instructionBrush;
        private SolidBrush immediateBrush;
        private SolidBrush absoluteBrush;
        private SolidBrush currentInstructionBrush;
        
        private int padding;
        private int splitX;
        private float bytesX;
        private int splitX2;
        private float instructionX;
        private int lineHeight;

        public DisassemblyControl()
        {
            this.DoubleBuffered = true;
            this.currentInstructionBrush = new SolidBrush(Color.Olive);
            this.addressBrush = new SolidBrush(Color.DimGray);
            this.instructionBrush = new SolidBrush(Color.LightGray);
            this.immediateBrush = new SolidBrush(Color.Chocolate);
            this.absoluteBrush = new SolidBrush(Color.Firebrick);
            this.seperatorPen = new Pen(new SolidBrush(Color.DimGray));
        }

        protected override void Dispose(bool disposing)
        {
            if (disposing)
            {
                this.currentInstructionBrush.Dispose();
                this.addressBrush.Dispose();
                this.seperatorPen.Dispose();
                this.instructionBrush.Dispose();
                this.immediateBrush.Dispose();
                this.absoluteBrush.Dispose();
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

        public ushort StartAddress
        {
            get
            {
                return this.startAddress;
            }
            set
            {
                this.startAddress = value;
                Invalidate();
            }
        }

        public ushort ProgramCounter
        {
            get
            {
                return this.programCounter;
            }
            set
            {
                if (this.programCounter != value)
                {
                    this.programCounter = value;
                    this.Invalidate();
                }
            }
        }

        public int InstructionCount
        {
            get
            {
                return this.instructionCount;
            }
            set
            {
                this.instructionCount = value;
                Invalidate();
            }
        }

        protected override void OnFontChanged(EventArgs e)
        {
            this.padding = this.Font.Height / 2;

            var linePadding = this.Font.Height / 4;
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
            this.bytesX = splitX + padding;
            var bytesSize = graphics.MeasureString("00 00 00", this.Font).Width;
            this.splitX2 = (int)(bytesX + bytesSize); 
            this.instructionX = splitX2 + padding;

            var y = padding;
            var currentAddress = this.startAddress;
            for (var i = 0; i < this.instructionCount; i++)
            {
                this.DrawInstruction(graphics, y, ref currentAddress);

                y += lineHeight;
            }

            graphics.DrawLine(this.seperatorPen, this.splitX, padding, this.splitX, y - lineHeight + this.Font.Height);
            graphics.DrawLine(this.seperatorPen, this.splitX2, padding, this.splitX2, y - lineHeight + this.Font.Height);

            base.OnPaint(e);
        }

        private void DrawInstruction(Graphics graphics, int y, ref ushort currentAddress)
        {
            if (currentAddress == this.programCounter)
            {
                graphics.FillRectangle(this.currentInstructionBrush, splitX2, y, this.Width -  splitX - padding, this.FontHeight);
            }

            graphics.DrawString("0x" + currentAddress.ToString("X4"), this.Font, this.addressBrush, padding, y);

            var opCode = this.bus.Read(currentAddress++);

            var byteX = this.bytesX;
            var byteWidth = graphics.MeasureString("00 ", this.Font).Width;
            graphics.DrawString(opCode.ToString("X2"), this.Font, this.addressBrush, byteX, y);
            var addressByteCount = InstructionDecoder.GetAddressingBytes(opCode);
            for (var i=0; i < addressByteCount; i++)
            {
                byteX += byteWidth;
                var nextByte = this.bus.Read((ushort)(currentAddress + i));
                graphics.DrawString(nextByte.ToString("X2"), this.Font, this.addressBrush, byteX, y);
            }

            var operation = InstructionDecoder.GetOperation(opCode);

            var opCodeWidth = graphics.MeasureString(operation + " ", this.Font).Width;
            graphics.DrawString(operation, this.Font, this.instructionBrush, instructionX, y);

            var argX = instructionX + opCodeWidth;

            var addressingMode = InstructionDecoder.GetAddressingMode(opCode);
            switch (addressingMode)
            {
                case AddressingMode.Implicit:
                    break;

                case AddressingMode.Immediate:
                    {
                        var value = this.bus.Read(currentAddress++);
                        graphics.DrawString("#$" + value.ToString("X2"), this.Font, this.immediateBrush, argX, y);
                        break;
                    }

                case AddressingMode.Absolute:
                    {
                        // TODO: highlight the jump target
                        var value = this.bus.Read(currentAddress++) | this.bus.Read(currentAddress++) << 8;
                        graphics.DrawString("$" + value.ToString("X4"), this.Font, this.absoluteBrush, argX, y);
                        break;
                    }

                case AddressingMode.Relative:
                    {
                        var relative = (sbyte)this.bus.Read(currentAddress++);
                        var value = currentAddress + relative;
                        graphics.DrawString("$" + value.ToString("X4"), this.Font, this.absoluteBrush, argX, y);
                        break;
                    }

                case AddressingMode.ZeroPage:
                    {
                        var value = this.bus.Read(currentAddress++);
                        graphics.DrawString("$" + value.ToString("X2"), this.Font, this.absoluteBrush, argX, y);
                        break;
                    }

                case AddressingMode.IndirectIndexed:
                    {
                        var value = this.bus.Read(currentAddress++);
                        graphics.DrawString("(", this.Font, this.instructionBrush, argX, y);
                        argX += graphics.MeasureString("(", this.Font).Width;
                        var address = "$" + value.ToString("X2");
                        graphics.DrawString(address, this.Font, this.absoluteBrush, argX, y);
                        argX += graphics.MeasureString(address, this.Font).Width;
                        graphics.DrawString("),Y", this.Font, this.instructionBrush, argX, y);
                        break;
                    }

                default:
                    currentAddress += (ushort)InstructionDecoder.GetAddressingBytes(opCode);
                    graphics.DrawString("...", this.Font, this.instructionBrush, argX, y);
                    break;
            }
        }
    }
}
