using System;

namespace NesEmu.Emulator
{
    public class Ppu
    {
        private Bus bus;
        private Display display;

        // approx!
        public int FrameCount = 0;

        private byte ppuStatus;
        private byte ppuControl;
        private byte ppuMask;

        private bool addressLatch;

        // the bits in the address registers can be viewed as 0yyy NNYY YYYX XXXX
        private ushort currentAddress;
        private ushort initialAddress;

        private byte ppuData;

        private byte[] palette = new byte[32];

        public int currentScanLine = -1;
        public int scanlineCycle = -1;

        private byte nextTileId;
        private byte nextPatternByteLow;
        private byte nextPatternByteHigh;

        private ushort patternShiftHigh;
        private ushort patternShiftLow;
        private uint attributeShift;
        private ushort nextAttributeShift;
        private byte currentPixel;
        private bool pixelRendered;

        // cache for code performance
        private int patternMask;
        private int patternBitShift;
        private int attributeMask;
        private int attributeBitShift;
        private ushort backgroundPatternBase;
        private ushort spritePatternBase;
        private ushort patternAddress;

        private short oamAddress;
        private byte oamData;
        private byte[] oam = new byte[256];
        private byte[] oamCopy = new byte[32];
        private byte oamCopyIndex;

        private int spriteIndex = 0;
        private Sprite[] sprites = new Sprite[8];
        private bool sprite0Selected;
        private bool sprite0Visible;
        private int scanlineSpriteCount;

        public Ppu(Bus bus, Display display)
        {
            this.bus = bus;
            this.display = display;
        }

        public byte[] Palette => this.palette;

        public void Tick()
        {
            this.DoTick();
            this.DoTick();
            this.DoTick();
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
            else if (address == 0x07)
            {
                var data = this.ppuData;
                var ppuAddress = (ushort)(this.currentAddress & 0x3fff);
                this.ppuData = this.bus.PpuRead(ppuAddress);

                if (ppuAddress >= 0x3f00)
                {
                    data = this.palette[ppuAddress & 0x1f];
                }

                if ((this.ppuControl & 0x04) != 0)
                {
                    this.currentAddress += 32;
                }
                else
                {
                    this.currentAddress += 1;
                }

                return data;
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

                    this.backgroundPatternBase = (ushort)((this.ppuControl & 0x10) << 8);
                    this.spritePatternBase = (ushort)((this.ppuControl & 0x08) << 9);

                    this.initialAddress &= 0xf3ff;
                    this.initialAddress |= (ushort)((value & 3) << 10);
                    return;

                case 1:
                    this.ppuMask = value;
                    return;

                case 3:
                    this.oamAddress = value;
                    return;

                case 5:
                    if (!this.addressLatch)
                    {
                        this.initialAddress &= 0xffe0;
                        this.initialAddress |= (byte)(value >> 3);
                        this.SetFineX((byte)(value & 7));
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
                        var writeAddress = (ushort)(this.currentAddress & 0x3fff);
                        if (writeAddress >= 0x3f00)
                        {
                            if ((writeAddress & 0x03) == 0)
                            {
                                // zero colors are mirrored
                                this.palette[writeAddress & 0x000f] = value;
                                this.palette[(writeAddress & 0x000f) | 0x0010] = value;
                            }
                            else
                            {
                                this.palette[writeAddress & 0x001f] = value;
                            }
                        }
                        else
                        {
                            this.bus.PpuWrite(writeAddress, value);
                        }

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

        internal void DmaWrite(byte value)
        {
            this.oam[this.oamAddress++] = value;
        }

        private void SetFineX(byte value)
        {
            this.patternBitShift = 15 - value;
            this.patternMask = 1 << patternBitShift;

            // 2 off since this maps to bits 2 and 3
            this.attributeBitShift = 28 - 2 * value;
            this.attributeMask = 3 << attributeBitShift + 2;
        }

        private void DoTick()
        {
            if (scanlineCycle == 0)
            {
                if (currentScanLine == 241)
                {
                    this.display.VBlank();

                    this.FrameCount++;

                    if ((this.ppuControl & 0x80) != 0)
                    {
                        this.bus.SignalNmi();
                    }

                    // The VBlank flag of the PPU is set at tick 1 (the second tick) of scanline 241
                    ppuStatus |= 0x80;
                }
                else if (currentScanLine == -1)
                {
                    ppuStatus &= 0x1f;
                }
            }

            if (scanlineCycle == -1)
            {
                scanlineCycle++;
                return;
            }


            if (this.currentScanLine < 240)
            {
                if (this.scanlineCycle < 256)
                {
                    if (this.currentScanLine >= 0)
                    {
                        this.pixelRendered = false;
                        this.currentPixel = palette[0];

                        if ((this.ppuMask & 0x08) != 0)
                        {
                            this.BackgroundRender();
                        }

                        if ((this.ppuMask & 0x10) != 0)
                        {
                            this.SpriteRender();
                            this.SpriteTick();
                        }

                        this.display.WritePixel(this.currentPixel);
                    }

                    if ((this.ppuMask & 0x18) != 0)
                    {
                        this.BackgroundLoadTick();
                        this.BackgroundTick();
                        this.SpriteEvaluationTick();
                    }
                }
                else if (this.scanlineCycle == 256)
                {
                    this.scanlineSpriteCount = this.oamCopyIndex >> 2;
                    this.sprite0Visible = this.sprite0Selected;
                    this.sprite0Selected = false;
                    this.oamAddress = 0;
                    this.oamCopyIndex = 0;
                    this.spriteIndex = 0;

                    this.display.HBlank();

                    if ((this.ppuMask & 0x18) != 0)
                    {
                        this.BackgroundHReset();
                    }
                }
                else if (this.scanlineCycle >= 256 && this.scanlineCycle < 320)
                {
                    if ((this.ppuMask & 0x18) != 0)
                    {
                        // sprite tile loading
                        this.SpriteLoadTick();
                    }

                    if (this.currentScanLine < 0 && (this.scanlineCycle >= 279 && this.scanlineCycle < 304))
                    {
                        if ((this.ppuMask & 0x18) != 0)
                        {
                            this.BackgroundVReset();
                        }
                    }
                }
                else if (this.scanlineCycle >= 320 && this.scanlineCycle < 336)
                {
                    if ((this.ppuMask & 0x18) != 0)
                    {
                        this.BackgroundLoadTick();
                        this.BackgroundTick();
                    }
                }
            }

            this.scanlineCycle++;
            if (this.scanlineCycle == 340)
            {
                this.scanlineCycle = -1;

                this.currentScanLine++;

                if (this.currentScanLine == 261)
                {
                    this.currentScanLine = -1;
                }
            }
        }

        private void BackgroundRender()
        {
            var index = (byte)((this.patternShiftHigh & this.patternMask) >> (this.patternBitShift - 1)
                             | (this.patternShiftLow & this.patternMask) >> this.patternBitShift);

            if (index != 0)
            {
                pixelRendered = true;

                index |= (byte)((this.attributeShift & this.attributeMask) >> attributeBitShift); // palette
                this.currentPixel = this.palette[index];
            }
        }

        private void BackgroundLoadTick()
        {
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

                    // we've determined the palette index.  Lets duplicate this 8 times for the tile pixels
                    this.nextAttributeShift = attributes;
                    this.nextAttributeShift |= (ushort)(nextAttributeShift << 2);
                    this.nextAttributeShift |= (ushort)(nextAttributeShift << 4);
                    this.nextAttributeShift |= (ushort)(nextAttributeShift << 8);
                    break;

                case 5:
                    // address is 000PTTTTTTTT0YYY
                    this.patternAddress = (ushort)
                        (backgroundPatternBase | // pattern selector
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
        }

        private void BackgroundTick()
        {
            this.patternShiftHigh <<= 1;
            this.patternShiftLow <<= 1;
            this.attributeShift <<= 2;
        }

        private void BackgroundVReset()
        {
            this.currentAddress &= 0x041f;
            this.currentAddress |= (ushort)(this.initialAddress & 0xfbe0);
        }

        private void BackgroundHReset()
        {
            this.currentAddress &= 0xfbe0;
            this.currentAddress |= (ushort)(this.initialAddress & 0x041f);
        }

        private void SpriteRender()
        {
            bool drawnSprite = false;

            for (var i = 0; i < this.scanlineSpriteCount; i++)
            {
                if (this.sprites[i].X != 0)
                {
                    continue;
                }

                byte index;
                if ((sprites[i].attributes & 0x40) == 0)
                {
                    index = (byte)((this.sprites[i].patternShiftHigh & 0x80) >> 6 | (this.sprites[i].patternShiftLow & 0x80) >> 7);

                    this.sprites[i].patternShiftHigh <<= 1;
                    this.sprites[i].patternShiftLow <<= 1;
                }
                else
                {
                    index = (byte)((this.sprites[i].patternShiftHigh & 0x01) << 1 | this.sprites[i].patternShiftLow & 0x01);

                    this.sprites[i].patternShiftHigh >>= 1;
                    this.sprites[i].patternShiftLow >>= 1;
                }

                if (index != 0 && !drawnSprite)
                {
                    drawnSprite = true;

                    if (this.pixelRendered)
                    {
                        if (i == 0 && this.sprite0Visible)
                        {
                            this.ppuStatus |= 0x40;
                        }

                        if ((sprites[i].attributes & 0x20) != 0)
                        {
                            continue;
                        }
                    }

                    index |= (byte)((0x04 | (this.sprites[i].attributes & 0x03)) << 2); // palette
                    this.currentPixel = this.palette[index];
                }
            }
        }

        private void SpriteTick()
        {
            for (var i=0; i<8; i++)
            {
                if (this.sprites[i].X > 0)
                {
                    this.sprites[i].X--;
                }
            }
        }

        private void SpriteEvaluationTick()
        {
            if ((this.scanlineCycle & 1) == 0)
            {
                // setting up the read/write
                return;
            }

            if (this.scanlineCycle < 64)
            {
                this.oamCopy[this.scanlineCycle/2] = this.oamData = 0xff;
                return;
            }

            if (this.oamAddress >= 256)
            {
                return;
            }

            this.oamData = this.oam[this.oamAddress];
            if (this.oamCopyIndex < 32)
            {
                this.oamCopy[this.oamCopyIndex] = this.oamData;

                if ((this.oamAddress & 0x03) != 0)
                {
                    this.oamAddress++;
                    this.oamCopyIndex++;
                    return;
                }
            }

            var spriteRow = (uint)(this.currentScanLine - this.oamData);
            bool visible = spriteRow < 8;

            if (this.oamAddress == 0)
            {
                this.sprite0Selected = visible;
            }

            if (visible)
            {
                if (oamCopyIndex >= 32)
                {
                    this.ppuStatus |= 0x20;
                    this.oamAddress += 4;
                }
                else
                {
                    this.oamAddress++;
                    this.oamCopyIndex++;
                }
            }
            else
            {
                this.oamAddress += 4;

                // TODO: overflow bug
            }
        }

        private void SpriteLoadTick()
        {
            if (this.spriteIndex >= this.scanlineSpriteCount)
            {
                return;
            }

            switch (scanlineCycle & 0x07)
            {
                case 5:
                    var oamAddress = this.spriteIndex << 2;

                    var attributes = this.oamCopy[oamAddress + 2];
                    this.sprites[this.spriteIndex].attributes = attributes;
                    this.sprites[this.spriteIndex].X = this.oamCopy[oamAddress + 3];

                    var tileId = this.oamCopy[oamAddress + 1];

                    var tileFineY = this.currentScanLine - this.oamCopy[oamAddress];

                    if ((attributes & 0x80) != 0)
                    {
                        tileFineY = 7 - tileFineY;
                    }

                    // address is 000PTTTTTTTT0YYY
                    this.patternAddress = (ushort)
                        (spritePatternBase | // pattern selector
                        (tileId << 4) |
                        tileFineY); 
                    this.sprites[this.spriteIndex].patternShiftLow = this.bus.PpuRead(this.patternAddress);

                    break;

                case 7:
                    // address is 000PTTTTTTTT1YYY
                    this.sprites[this.spriteIndex].patternShiftHigh = this.bus.PpuRead((ushort)(this.patternAddress | 8));
                    this.spriteIndex++;
                    break;
            }
        }

        private struct Sprite
        {
            public byte X;
            public byte patternShiftHigh;
            public byte patternShiftLow;
            public byte attributes;
        }
    }
}
