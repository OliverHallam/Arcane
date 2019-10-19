using System;
using System.Collections.Generic;
using System.IO;
using System.Text;

namespace NesEmu.Emulator
{
    public class Cart
    {
        private byte[] prgBankA;
        private byte[] prgBankB;

        private byte[] chrData;


        public Cart(byte[] prgBankA, byte[] prgBankB, byte[] chrData)
        {
            this.prgBankA = prgBankA;
            this.prgBankB = prgBankB;
            this.chrData = chrData;
        }

        public bool Read(ushort address, out byte value)
        {
            if (address >= 0xC000)
            {
                value = prgBankB[address & 0x3fff];
                return true;
            }

            if (address >= 0x8000)
            {
                value = prgBankA[address & 0x3fff];
                return true;
            }

            value = 0;
            return false;
        }

        public static Cart Load(Stream data)
        {
            var cart = TryLoad(data);
            if (cart == null)
            {
                throw new Exception("The file was invalid");
            }

            return cart;
        }

        private static Cart TryLoad(Stream data)
        {
            var header = new byte[16];
            if (data.Read(header, 0, 16) != 16)
            {
                return null;
            }

            if (header[0] != 'N' || header[1] != 'E' || header[2] != 'S' || header[3] != 0x1a)
            {
                return null;
            }

            var prgCount = header[4];
            var chrSize = header[5] * 0x2000;

            var flags6 = header[6];

            var mapper = flags6 >> 4;

            var hasTrainer = (flags6 & 0x04) != 0;

            if (hasTrainer)
            {
                data.Seek(512, SeekOrigin.Current);
            }

            var prgBanks = new byte[prgCount][];
            for (var i = 0; i < prgCount; i++)
            {
                var prgBank = new byte[0x4000];
                if (data.Read(prgBank, 0, 0x4000) != 0x4000)
                {
                    return null;
                }
                prgBanks[i] = prgBank;
            }

            var chrData = new byte[chrSize];
            if (data.Read(chrData, 0, chrSize) != chrSize)
            {
                return null;
            }

            if (data.Position != data.Length)
            {
                return null;
            }

            switch (mapper)
            {
                case 0:
                    switch (prgBanks.Length)
                    {
                        case 1:
                            return new Cart(prgBanks[0], prgBanks[0], chrData);

                        case 2:
                            return new Cart(prgBanks[0], prgBanks[1], chrData);
                    }
                    break;
            }

            return null;
        }
    }
}
