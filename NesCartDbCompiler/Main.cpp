#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>

#include "CartRecord.h"
#include "Main.h"

std::vector<std::string> ReadCsvLine(std::istream& stream)
{
    std::vector<std::string> result;

    std::string line;
    std::getline(stream, line);

    std::stringstream lineStream(line);

    while (lineStream.peek() == ' ')
        lineStream.ignore();

    auto peek = lineStream.peek();
    if (peek == '#' || peek == '\r' || peek == '\n')
        return {};

    std::string cell;
    while (true)
    {
        if (lineStream.peek() == '"')
        {
            lineStream.ignore();

            std::getline(lineStream, cell, '"');

            while (lineStream.peek() == ' ')
                lineStream.ignore();

            auto next = lineStream.get();
            if (next >= 0 && next != ',')
                return {};
        }
        else
        {
            if (!std::getline(lineStream, cell, ','))
                break;

            auto trimIndex = cell.find_last_not_of(' ');

            if (trimIndex != std::string::npos)
                cell = cell.substr(0, trimIndex + 1);
            else
                cell.clear();
        }

        result.push_back(cell);

        while (stream.peek() == ' ')
            stream.ignore();
    }

    return result;
}

int SizeToInt(std::string sizeString)
{
    auto last = sizeString.back();
    if (last == 'K')
        return std::stoi(sizeString.substr(0, sizeString.length() - 1)) * 1024;

    return std::stoi(sizeString);
}

bool ParseMapper(const std::string& mapperString, uint16_t& mapper, uint8_t& subMapper)
{
    auto mapperDotIndex = mapperString.find_first_of('.');
    if (mapperDotIndex == std::string::npos)
        return false;

    mapper = std::stoi(mapperString.substr(0, mapperDotIndex));
    subMapper = std::stoi(mapperString.substr(mapperDotIndex + 1));
    return true;
}

bool ParseMirrorFlag(const std::string& mirrorFlag, uint8_t& mode)
{
    bool mirrorModeObserved = false;
    bool batteryObserved = false;

    for (auto c : mirrorFlag)
    {
        if (c == 'H' && !mirrorModeObserved)
        {
            mirrorModeObserved = true;
            mode = 1;
        }
        else if (c == 'V' && !mirrorModeObserved)
        {
            mirrorModeObserved = true;
            mode = 2;
        }
        else if (c == '4')
        {
            mirrorModeObserved = true;
            mode = 3;
        }
        else if (c == 'B' && !batteryObserved)
        {
            batteryObserved = true;
        }
        else
        {
            return false;
        }
    }

    return true;
}

std::vector<CartRecord> ReadCsvData()
{
    std::ifstream csvStream(L"CartData.csv");

    std::vector<CartRecord> carts;

    while (csvStream.good())
    {
        auto row = ReadCsvLine(csvStream);
        if (!row.size())
            continue;

        if (row.size() != 14)
            return {};

        CartRecord record;

        record.Name = row[0];
        record.PrgRom = std::stoi(row[1]) * 1024;
        record.ChrRom = std::stoi(row[2]) * 1024;
        record.MiscRom = std::stoi(row[3]);
        if (!ParseMapper(row[4], record.Mapper, record.SubMapper))
            return {};
        record.MirrorFlag = row[5];
        record.Ppu = row[6];
        record.PrgRam = SizeToInt(row[7]);
        record.PrgRamB = SizeToInt(row[8]);
        record.ChrRam = SizeToInt(row[9]);
        record.ChrRamB = SizeToInt(row[10]);
        record.Console = row[11];
        record.Controller = row[12];
        record.Crc32 = std::stoul(row[13], nullptr, 16);

        carts.emplace_back(std::move(record));
    }

    return carts;
}

bool WriteBinaryData(const std::vector<CartRecord> & carts)
{
    std::ofstream binaryData(L"CartData.dat", std::ios::binary);

    for (auto& cart : carts)
    {
        binaryData.write(reinterpret_cast<const char*>(&cart.Crc32), sizeof(uint32_t));

        // we'll assume the Rom sizes were right, otherwise the iNes file was invalid

        // we can probably cut this back to a few bits based on the mappers we support
        if (cart.Mapper > 256)
            return false;
        auto mapperByte = static_cast<uint8_t>(cart.Mapper);
        binaryData.write(reinterpret_cast<const char*>(&mapperByte), sizeof(uint8_t));

        // next byte:
        // sssm mcbr
        // |||| |||+ 8K PRG-RAM
        // |||| ||+- 8K Battery backed PRG RAM
        // |||| |+-- 8K CHR RAM
        // |||+-+--- Mirror Mode
        // +++------ Sub mapper / prg-ram size override
        if (cart.SubMapper > 15)
            return false;

        if (cart.PrgRam != 0 && cart.PrgRam != 8 * 1024)
            return false;

        auto subBits = cart.SubMapper;

        auto expectedPrgRamSize = 8 * 1024;
        if (cart.Mapper == 4 && cart.SubMapper == 1)
        {
            // For sub-mapper 1, we assume 1k of PRG-RAM B
            if (cart.PrgRamB != 0 && cart.PrgRamB != 1 * 1024)
                return false;
        }
        else if (cart.Mapper == 5)
        {
            if (cart.SubMapper != 0)
                return false;

            // we can repurpose the submapper bits to override the size
            if (cart.PrgRamB == 0 || cart.PrgRamB == 8 * 1024)
                subBits = 0;
            else if (cart.PrgRamB == 1024)
                subBits = 1;
            else if (cart.PrgRamB == 32 * 1024)
                subBits = 2;
            else
                return false;
        }
        else if (cart.PrgRamB != 0 && cart.PrgRamB != 8 * 1024)
            return false;

        if (cart.Mapper == 13)
        {
            if (cart.ChrRam != 16 * 1024)
                return false;
        }
        else if (cart.ChrRam != 0 && cart.ChrRam != 8 * 1024)
            return false;

        if (cart.ChrRamB != 0)
            return false;

        uint8_t mirrorMode;
        if (!ParseMirrorFlag(cart.MirrorFlag, mirrorMode))
            return false;

        uint8_t data = 0;
        if (cart.PrgRam)
            data |= 0x01;
        if (cart.PrgRamB)
            data |= 0x02;
        if (cart.ChrRam)
            data |= 0x04;
        data |= mirrorMode << 3;
        data |= subBits << 4;

        binaryData.write(reinterpret_cast<const char*>(&data), sizeof(uint8_t));
    }

    return true;
}

void WriteHeader()
{
    std::ofstream headerStream(L"CartData.h", std::ios::binary);
    std::ifstream dataStream(L"CartData.dat", std::ios::binary);

    headerStream << "#include <cstdint>" << std::endl;
    headerStream << std::endl;
    headerStream << "const uint8_t CartData[] = " << std::endl;
    headerStream << "{" << std::endl;

    headerStream << std::hex;

    while (dataStream.good() && dataStream.peek() >= 0)
    {
        headerStream << "    ";
        for (auto i = 0; i < 6; i++)
        {
            auto byte = dataStream.get();
            headerStream << "0x" << std::setfill('0') << std::setw(2) << byte;

            if (dataStream.peek() < 0)
                break;

            headerStream << ", ";
        }

        headerStream << std::endl;
    }

    headerStream << "};" << std::endl;
}

int main(int argc, char* argv[])
{
    auto carts = ReadCsvData();
    if (!carts.size())
        return -1;

    if (!WriteBinaryData(carts))
        return -1;

    WriteHeader();

    return 0;
}
