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
        private byte nextAttributeByte;
        private byte nextPatternByteLow;
        private byte nextPatternByteHigh;

        // the bits address register can be viewed as 0tttNNYYYYYXXXXX
        private byte fineX;
        private ushort currentAddress;
        private ushort initialAddress;

        private ushort patternShiftHigh;
        private ushort patternShiftLow;
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
                    if (currentScanLine >= 0 && scanlineCycle < 256)
                    {
                        // draw the pixel
                        var value = (byte)((this.patternShiftHigh & 0x80) >> 6 | (this.patternShiftLow & 0x80) >> 7);
                        this.Frame[this.currentPixelAddress++] = value;
                        this.patternShiftHigh <<= 1;
                        this.patternShiftLow <<= 1;
                    }

                    if (scanlineCycle < 256)
                    {
                        switch (scanlineCycle & 0x07)
                        {
                            case 0:
                                if (scanlineCycle != 0)
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

                            case 1:
                                this.patternShiftHigh |= (ushort)(this.nextPatternByteHigh);
                                this.patternShiftLow |= (ushort)(this.nextPatternByteLow);
                                this.nextTileId = this.bus.PpuRead((ushort)(0x2000 | this.currentAddress & 0x0fff));
                                break;

                            case 3:
                                // TODO: attribute address = 0x23C0 | (v & 0x0C00) | ((v >> 4) & 0x38) | ((v >> 2) & 0x07)
                                this.nextAttributeByte = 0;
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
                                break;
                        }

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
                    }
                    else if (scanlineCycle == 256)
                    {
                        this.currentAddress &= 0xffe0;
                        this.currentAddress |= (ushort)(this.initialAddress & 0x001f);
                    }
                    else if (currentScanLine < 0 && (scanlineCycle >= 279 && scanlineCycle < 304))
                    {
                        this.currentAddress = this.initialAddress;
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
