using System;
using System.Collections.Generic;
using System.Drawing;
using System.Drawing.Imaging;
using System.Text;

namespace NesEmu
{
    class NesColorPalette
    {
        public static void SetPalette(ColorPalette palette)
        {
            palette.Entries[0] = Color.FromArgb(124, 124, 124);
            palette.Entries[1] = Color.FromArgb(0, 0, 252);
            palette.Entries[2] = Color.FromArgb(0, 0, 188);
            palette.Entries[3] = Color.FromArgb(68, 40, 188);
            palette.Entries[4] = Color.FromArgb(148, 0, 132);
            palette.Entries[5] = Color.FromArgb(168, 0, 32);
            palette.Entries[6] = Color.FromArgb(168, 16, 0);
            palette.Entries[7] = Color.FromArgb(136, 20, 0);
            palette.Entries[8] = Color.FromArgb(80, 48, 0);
            palette.Entries[9] = Color.FromArgb(0, 120, 0);
            palette.Entries[10] = Color.FromArgb(0, 104, 0);
            palette.Entries[11] = Color.FromArgb(0, 88, 0);
            palette.Entries[12] = Color.FromArgb(0, 64, 88);
            palette.Entries[13] = Color.FromArgb(0, 0, 0);
            palette.Entries[14] = Color.FromArgb(0, 0, 0);
            palette.Entries[15] = Color.FromArgb(0, 0, 0);
            palette.Entries[16] = Color.FromArgb(188, 188, 188);
            palette.Entries[17] = Color.FromArgb(0, 120, 248);
            palette.Entries[18] = Color.FromArgb(0, 88, 248);
            palette.Entries[19] = Color.FromArgb(104, 68, 252);
            palette.Entries[10] = Color.FromArgb(216, 0, 204);
            palette.Entries[21] = Color.FromArgb(228, 0, 88);
            palette.Entries[22] = Color.FromArgb(248, 56, 0);
            palette.Entries[23] = Color.FromArgb(228, 92, 16);
            palette.Entries[24] = Color.FromArgb(172, 124, 0);
            palette.Entries[25] = Color.FromArgb(0, 184, 0);
            palette.Entries[26] = Color.FromArgb(0, 168, 0);
            palette.Entries[27] = Color.FromArgb(0, 168, 68);
            palette.Entries[28] = Color.FromArgb(0, 136, 136);
            palette.Entries[29] = Color.FromArgb(0, 0, 0);
            palette.Entries[30] = Color.FromArgb(0, 0, 0);
            palette.Entries[31] = Color.FromArgb(0, 0, 0);
            palette.Entries[32] = Color.FromArgb(248, 248, 248);
            palette.Entries[33] = Color.FromArgb(60, 188, 252);
            palette.Entries[34] = Color.FromArgb(104, 136, 252);
            palette.Entries[35] = Color.FromArgb(152, 120, 248);
            palette.Entries[36] = Color.FromArgb(248, 120, 248);
            palette.Entries[37] = Color.FromArgb(248, 88, 152);
            palette.Entries[38] = Color.FromArgb(248, 120, 88);
            palette.Entries[39] = Color.FromArgb(252, 160, 68);
            palette.Entries[40] = Color.FromArgb(248, 184, 0);
            palette.Entries[41] = Color.FromArgb(184, 248, 24);
            palette.Entries[42] = Color.FromArgb(88, 216, 84);
            palette.Entries[43] = Color.FromArgb(88, 248, 152);
            palette.Entries[44] = Color.FromArgb(0, 232, 216);
            palette.Entries[45] = Color.FromArgb(120, 120, 120);
            palette.Entries[46] = Color.FromArgb(0, 0, 0);
            palette.Entries[47] = Color.FromArgb(0, 0, 0);
            palette.Entries[48] = Color.FromArgb(252, 252, 252);
            palette.Entries[49] = Color.FromArgb(164, 228, 252);
            palette.Entries[50] = Color.FromArgb(184, 184, 248);
            palette.Entries[51] = Color.FromArgb(216, 184, 248);
            palette.Entries[52] = Color.FromArgb(248, 184, 248);
            palette.Entries[53] = Color.FromArgb(248, 164, 192);
            palette.Entries[54] = Color.FromArgb(240, 208, 176);
            palette.Entries[55] = Color.FromArgb(252, 224, 168);
            palette.Entries[56] = Color.FromArgb(248, 216, 120);
            palette.Entries[57] = Color.FromArgb(216, 248, 120);
            palette.Entries[58] = Color.FromArgb(184, 248, 184);
            palette.Entries[59] = Color.FromArgb(184, 248, 216);
            palette.Entries[60] = Color.FromArgb(0, 252, 252);
            palette.Entries[61] = Color.FromArgb(248, 216, 248);
            palette.Entries[62] = Color.FromArgb(0, 0, 0);
            palette.Entries[63] = Color.FromArgb(0, 0, 0);
        }
    }
}
