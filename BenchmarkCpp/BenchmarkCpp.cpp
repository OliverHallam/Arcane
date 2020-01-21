#include <chrono>
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>

#include "..\NesCoreCpp\NesSystem.h"

int main()
{
    auto path = R"(c:\roms\NESRoms\World\Super Mario Bros (JU) (PRG 0).nes)";
    auto Frames = 10000;

    std::ifstream file(path, std::ios::binary | std::ios::ate);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> buffer(size);
    if (!file.read(buffer.data(), size))
    {
        return -1;
    }

    auto system = std::make_unique<NesSystem>();

    auto cart = TryLoadCart(reinterpret_cast<uint8_t *>(&buffer[0]), buffer.size());

    system->InsertCart(std::move(cart));
    system->Reset();

    auto startTime = std::chrono::high_resolution_clock::now();

    for (auto i = 0; i < Frames; i++)
    {
        system->RunFrame();
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    std::cout << elapsed.count() << "ms";
}
