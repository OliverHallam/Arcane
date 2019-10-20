using System;

namespace NesEmu.Emulator
{
    public class InstructionDecoder
    {
        private static string[] operationNames = new string[256]
        {
            "BRK", "ORA", "KIL", "SLO", "NOP", "ORA", "ASL", "SLO", "PHP", "ORA", "ASL", "ANC", "NOP", "ORA", "ASL", "SLO",
            "BPL", "ORA", "KIL", "SLO", "NOP", "ORA", "ASL", "SLO", "CLC", "ORA", "NOP", "SLO", "NOP", "ORA", "ASL", "SLO",
            "JSR", "AND", "KIL", "RLA", "BIT", "AND", "ROL", "RLA", "PLP", "AND", "ROL", "ANC", "BIT", "AND", "ROL", "RLA",
            "BMI", "AND", "KIL", "RLA", "NOP", "AND", "ROL", "RLA", "SEC", "AND", "NOP", "RLA", "NOP", "AND", "ROL", "RLA",
            "RTI", "EOR", "KIL", "SRE", "NOP", "EOR", "LSR", "SRE", "PHA", "EOR", "LSR", "ALR", "JMP", "EOR", "LSR", "SRE",
            "BVC", "EOR", "KIL", "SRE", "NOP", "EOR", "LSR", "SRE", "CLI", "EOR", "NOP", "SRE", "NOP", "EOR", "LSR", "SRE",
            "RTS", "ADC", "KIL", "RRA", "NOP", "ADC", "ROR", "RRA", "PLA", "ADC", "ROR", "ARR", "JMP", "ADC", "ROR", "RRA",
            "BVS", "ADC", "KIL", "RRA", "NOP", "ADC", "ROR", "RRA", "SEI", "ADC", "NOP", "RRA", "NOP", "ADC", "ROR", "RRA",
            "NOP", "STA", "NOP", "SAX", "STY", "STA", "STX", "SAX", "DEY", "NOP", "TXA", "XAA", "STY", "STA", "STX", "SAX",
            "BCC", "STA", "KIL", "AHX", "STY", "STA", "STX", "SAX", "TYA", "STA", "TXS", "TAS", "SHY", "STA", "SHX", "AHX",
            "LDY", "LDA", "LDX", "LAX", "LDY", "LDA", "LDX", "LAX", "TAY", "LDA", "TAX", "LAX", "LDY", "LDA", "LDX", "LAX",
            "BCS", "LDA", "KIL", "LAX", "LDY", "LDA", "LDX", "LAX", "CLV", "LDA", "TSX", "LAS", "LDY", "LDA", "LDX", "LAX",
            "CPY", "CMP", "NOP", "DCP", "CPY", "CMP", "DEC", "DCP", "INY", "CMP", "DEX", "AXS", "CPY", "CMP", "DEC", "DCP",
            "BNE", "CMP", "KIL", "DCP", "NOP", "CMP", "DEC", "DCP", "CLD", "CMP", "NOP", "DCP", "NOP", "CMP", "DEC", "DCP",
            "CPX", "SBC", "NOP", "ISC", "CPX", "SBC", "INC", "ISC", "INX", "SBC", "NOP", "SBC", "CPX", "SBC", "INC", "ISC",
            "BEQ", "SBC", "KIL", "ISC", "NOP", "SBC", "INC", "ISC", "SED", "SBC", "NOP", "ISC", "NOP", "SBC", "INC", "ISC"
        };

        private static int[] addressingBytes = new int[256]
        {
            0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 2, 2, 2, 2,
            1, 1, 0, 1, 1, 1, 1, 1, 0, 2, 0, 2, 2, 2, 2, 2,
            2, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 2, 2, 2, 2,
            1, 1, 0, 1, 1, 1, 1, 1, 0, 2, 0, 2, 2, 2, 2, 2,
            0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 2, 2, 2, 2,
            1, 1, 0, 1, 1, 1, 1, 1, 0, 2, 0, 2, 2, 2, 2, 2,
            0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 2, 2, 2, 2,
            1, 1, 0, 1, 1, 1, 1, 1, 0, 2, 0, 2, 2, 2, 2, 2,
            1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 2, 2, 2, 2,
            1, 1, 0, 1, 1, 1, 1, 1, 0, 2, 0, 2, 2, 2, 2, 2,
            1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 2, 2, 2, 2,
            1, 1, 0, 1, 1, 1, 1, 1, 0, 2, 0, 2, 2, 2, 2, 2,
            1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 2, 2, 2, 2,
            1, 1, 0, 1, 1, 1, 1, 1, 0, 2, 0, 2, 2, 2, 2, 2,
            1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 2, 2, 2, 2,
            1, 1, 0, 1, 1, 1, 1, 1, 0, 2, 0, 2, 2, 2, 2, 2
        };

        private static AddressingMode[] addressingModes = GetAddressingModes();

        public static string GetOperation(byte opCode)
        {
            return operationNames[opCode];
        }

        static AddressingMode[] GetAddressingModes()
        {
            const AddressingMode acc = AddressingMode.Accumulator;
            const AddressingMode imp = AddressingMode.Implicit;
            const AddressingMode imm = AddressingMode.Immediate;
            const AddressingMode zp = AddressingMode.ZeroPage;
            const AddressingMode zpx = AddressingMode.ZeroPageX;
            const AddressingMode zpy = AddressingMode.ZeroPageY;
            const AddressingMode izx = AddressingMode.IndexedIndirect;
            const AddressingMode izy = AddressingMode.IndirectIndexed;
            const AddressingMode ind = AddressingMode.Indirect;
            const AddressingMode abs = AddressingMode.Absolute;
            const AddressingMode abx = AddressingMode.AbsoluteX;
            const AddressingMode aby = AddressingMode.AbsoluteY;
            const AddressingMode rel = AddressingMode.Relative;

            return new AddressingMode[256]
            {
                imp, izx, imp, izx, zp,  zp,  zp,  zp,  imp, imm, acc, imm, abs, abs, abs, abs,
                rel, izy, imp, izy, zpx, zpx, zpx, zpx, imp, aby, imp, aby, abx, abx, abx, abx,
                abs, izx, imp, izx, zp,  zp,  zp,  zp,  imp, imm, acc, imm, abs, abs, abs, abs,
                rel, izy, imp, izy, zpx, zpx, zpx, zpx, imp, aby, imp, aby, abx, abx, abx, abx,
                imp, izx, imp, izx, zp,  zp,  zp,  zp,  imp, imm, acc, imm, abs, abs, abs, abs,
                rel, izy, imp, izy, zpx, zpx, zpx, zpx, imp, aby, imp, aby, abx, abx, abx, abx,
                imp, izx, imp, izx, zp,  zp,  zp,  zp,  imp, imm, acc, imm, ind, abs, abs, abs,
                rel, izy, imp, izy, zpx, zpx, zpx, zpx, imp, aby, imp, aby, abx, abx, abx, abx,
                imm, izx, imm, izx, zp,  zp,  zp,  zp,  imp, imm, imp, imm, abs, abs, abs, abs,
                rel, izy, imp, izy, zpx, zpx, zpy, zpy, imp, aby, imp, aby, abx, abx, aby, aby,
                imm, izx, imm, izx, zp,  zp,  zp,  zp,  imp, imm, imp, imm, abs, abs, abs, abs,
                rel, izy, imp, izy, zpx, zpx, zpy, zpy, imp, aby, imp, aby, abx, abx, aby, aby,
                imm, izx, imm, izx, zp,  zp,  zp,  zp,  imp, imm, imp, imm, abs, abs, abs, abs,
                rel, izy, imp, izy, zpx, zpx, zpx, zpx, imp, aby, imp, aby, abx, abx, abx, abx,
                imm, izx, imm, izx, zp,  zp,  zp,  zp,  imp, imm, imp, imm, abs, abs, abs, abs,
                rel, izy, imp, izy, zpx, zpx, zpx, zpx, imp, aby, imp, aby, abx, abx, abx, abx
            };
        }

        public static AddressingMode GetAddressingMode(byte opCode)
        {
            return addressingModes[opCode];
        }

        public static int GetAddressingBytes(byte opCode)
        {
            return addressingBytes[opCode];
        }
    }
}
