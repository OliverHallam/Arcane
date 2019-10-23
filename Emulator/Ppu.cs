using System;

namespace NesEmu.Emulator
{
    public class Ppu
    {
        private Bus bus;

        // approx!
        public int FrameCount = 0;

        private byte ppuStatus;
        private byte ppuControl;
        private byte ppuMask;

        private byte ppuData;

        private bool addressLatch;

        private byte nextTileId;
        private byte nextPatternByteLow;
        private byte nextPatternByteHigh;

        // the bits address register can be viewed as 0ttt NNYY YYYX XXXX
        private byte fineX;
        private ushort currentAddress;
        private ushort initialAddress;

        private ushort patternShiftHigh;
        private ushort patternShiftLow;
        private uint attributeShift;
        private ushort nextAttributeShift;
        private int currentPixelAddress;

        // cache for code performance
        private ushort patternAddress;

        public Ppu(Bus bus)
        {
            this.bus = bus;
        }

        public byte[] Frame { get; } = new byte[256 * 240];

        public int currentScanLine = -1;
        public int scanlineCycle = -1;

        public void Tick()
        {
            this.DoTick();
            this.DoTick();
            this.DoTick();
        }

        private void DoTick()
        {
            if (scanlineCycle == 0)
            {
                if (currentScanLine == 241)
                {
                    this.currentPixelAddress = 0;
                    this.FrameCount++;
                    this.OnFrame?.Invoke(this, EventArgs.Empty);

                    if ((this.ppuControl & 0x80) != 0)
                    {
                        this.bus.SignalNmi();
                    }

                    // The VBlank flag of the PPU is set at tick 1 (the second tick) of scanline 241
                    ppuStatus |= 0x80;
                }
                else if (currentScanLine == -1)
                {
                    ppuStatus &= 0x7f;
                }
            }

            if (scanlineCycle == -1)
            {
                scanlineCycle++;
                return;
            }

            if ((this.ppuMask & 0x08) != 0)
            {
                if (currentScanLine < 240)
                {
                    if (scanlineCycle < 256 || (scanlineCycle >= 320 && scanlineCycle < 336))
                    {
                        if (currentScanLine >= 0 && scanlineCycle < 256)
                        {
                            // draw the pixel
                            var index = (byte)((this.patternShiftHigh & 0x8000) >> 14 | (this.patternShiftLow & 0x8000) >> 15);
                            if (index != 0)
                            {
                                index |= (byte)((this.attributeShift & 0xC0000000) >> 28); // pallette
                            }

                            this.Frame[this.currentPixelAddress++] = this.bus.PpuRead((ushort)(0x3f00 + index));
                        }

                        switch (scanlineCycle & 0x07)
                        {
                            case 0:
                                this.patternShiftHigh |= this.nextPatternByteHigh;
                                this.patternShiftLow |= this.nextPatternByteLow;
                                this.attributeShift |= this.nextAttributeShift;
                                break;

                            case 1:
                                var tileAddress = (ushort)(0x2000 | this.currentAddress & 0x0fff);
                                this.nextTileId = this.bus.PpuRead(tileAddress);
                                break;

                            case 3:
                                var attributeAddress = (ushort)(
                                    0x2000 | (this.currentAddress & 0x0C00) | // select table
                                    0x03C0 | // attribute block at end of table
                                    (this.currentAddress >> 4) & 0x0038 | // 3 bits of tile y
                                    (this.currentAddress >> 2) & 0x0007); // 3 bits of tile x

                                var attributes = this.bus.PpuRead(attributeAddress);

                                // use one more bit of the tile x and y to get the quadrant
                                attributes >>= ((this.currentAddress & 0x0040) >> 4) | (this.currentAddress & 0x0002); 
                                attributes &= 0x03;
                                
                                // we've determined the pallette index.  Lets duplicate this 8 times for the tile pixels
                                this.nextAttributeShift = attributes;
                                this.nextAttributeShift |= (ushort)(nextAttributeShift << 2);
                                this.nextAttributeShift |= (ushort)(nextAttributeShift << 4);
                                this.nextAttributeShift |= (ushort)(nextAttributeShift << 8);
                                break;

                            case 5:
                                // address is 000PTTTTTTTT0YYY
                                this.patternAddress = (ushort)
                                    (((this.ppuControl & 0x10) << 8) | // pattern selector
                                     (this.nextTileId << 4) |
                                     (this.currentAddress >> 12)); // fineY

                                this.nextPatternByteLow = this.bus.PpuRead(this.patternAddress);
                                break;

                            case 7:
                                // address is 000PTTTTTTTT1YYY
                                this.nextPatternByteHigh = this.bus.PpuRead((ushort)(this.patternAddress | 8));

                                if (scanlineCycle == 255)
                                {
                                    // adjust y scroll
                                    this.currentAddress += 0x1000;
                                    this.currentAddress &= 0x7fff;

                                    if ((this.currentAddress & 0x7000) == 0)
                                    {
                                        // move to the next row
                                        if ((this.currentAddress & 0x03e0) == 0x03e0)
                                        {
                                            this.currentAddress &= 0x7c1f;
                                            this.currentAddress ^= 0x0800;
                                        }
                                        else
                                        {
                                            this.currentAddress += 0x0020;
                                        }
                                    }
                                }
                                else
                                {
                                    // increment the x part of the address
                                    if ((this.currentAddress & 0x001f) == 0x001f)
                                    {
                                        this.currentAddress &= 0xffe0;
                                        this.currentAddress ^= 0x0400;
                                    }
                                    else
                                    {
                                        this.currentAddress++;
                                    }
                                }
                                break;
                        }

                        this.patternShiftHigh <<= 1;
                        this.patternShiftLow <<= 1;
                        this.attributeShift <<= 2;
                    }
                    else if (scanlineCycle == 256)
                    {
                        this.currentAddress &= 0xffe0;
                        this.currentAddress |= (ushort)(this.initialAddress & 0x001f);
                    }
                    else if (currentScanLine < 0 && (scanlineCycle >= 279 && scanlineCycle < 304))
                    {
                        this.currentAddress &= 0x0c1f;
                        this.currentAddress |= (ushort)(this.initialAddress & 0xf3e0);
                    }
                }
            }

            scanlineCycle++;
            if (scanlineCycle == 340)
            {
                scanlineCycle = -1;

                currentScanLine++;

                if (currentScanLine == 261)
                {
                    currentScanLine = -1;
                }
            }
        }

        public byte Peek(ushort address)
        {
            address &= 0x07;

            if (address == 0x02)
            {
                return this.ppuStatus;
            }

            return 0;
        }

        public byte Read(ushort address)
        {
            address &= 0x07;

            if (address == 0x02)
            {
                var status = this.ppuStatus;

                this.ppuStatus &= 0x7f;
                this.addressLatch = false;

                return status;
            }

            return 0;
        }


        public void Write(ushort address, byte value)
        {
            address &= 0x07;
            switch (address)
            {
                case 0:
                    this.ppuControl = value;

                    this.initialAddress &= 0xf3ff;
                    this.initialAddress |= (ushort)(value & 3 << 10);
                    return;

                case 1:
                    this.ppuMask = value;
                    return;

                case 5:
                    if (!this.addressLatch)
                    {
                        this.initialAddress &= 0xffe0;
                        this.initialAddress |= (byte)(value >> 3);
                        this.fineX = (byte)(value & 7);
                        addressLatch = true;
                    }
                    else
                    {
                        this.initialAddress &= 0x8c1f;
                        this.initialAddress |= (ushort)((value & 0xf8) << 2);
                        this.initialAddress |= (ushort)((value & 0x07) << 12);
                        addressLatch = false;
                    }
                    return;

                case 6:
                    if (!this.addressLatch)
                    {
                        this.initialAddress = (ushort)(((value & 0x3f) << 8) | (this.initialAddress & 0xff));
                        addressLatch = true;
                    }
                    else
                    {
                        this.initialAddress = (ushort)((this.initialAddress & 0xff00) | value);
                        this.currentAddress = initialAddress;
                        addressLatch = false;
                    }
                    return;

                case 7:
                    {
                        this.bus.PpuWrite(this.currentAddress, value);

                        if ((this.ppuControl & 0x04) != 0)
                        {
                            this.currentAddress += 32;
                        }
                        else
                        {
                            this.currentAddress += 1;
                        }
                    }
                    return;

                default:
                    return;
            }
        }

        public event EventHandler OnFrame;
    }
}
