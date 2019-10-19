namespace NesEmu.Emulator
{
    class Bus
    {
        private byte[] memory = new byte[2048];

        public byte Read(short address)
        {
            return memory[address & 0x7ff];
        }

        public void Write(short address, byte value)
        {
            memory[address & 0x7ff] = value;
        }
    }
}
