#include "Bus.h"

#include "Cart.h"
#include "Controller.h"
#include "Cpu.h"
#include "Ppu.h"


Bus::Bus() :
    cpu_(nullptr),
    ppu_(nullptr),
    apu_(nullptr),
    controller_(nullptr),
    cycleCount_{0}
{
    cpuRam_.fill(0xff);
    ppuRam_.fill(0xff);
}

void Bus::Attach(Cpu* cpu)
{
    cpu_ = cpu;
}

void Bus::Attach(Ppu* ppu)
{
    ppu_ = ppu;
}

void Bus::Attach(Apu* apu)
{
    apu_ = apu;
}

void Bus::Attach(Controller* controller)
{
    controller_ = controller;
}

void Bus::Attach(std::unique_ptr<Cart> cart)
{
    cart_ = std::move(cart);
    cart_->Attach(this);
}

void Bus::TickCpu()
{
    cycleCount_++;

    apu_->Tick();
    ppu_->Tick3();
}

uint32_t Bus::CycleCount() const
{
    return cycleCount_;
}

void Bus::SyncPpu()
{
    ppu_->Sync();
}

uint8_t Bus::CpuReadData(uint16_t address)
{
    uint8_t value;

    if (address < 0x2000)
        value = cpuRam_[address & 0x7ff];
    else if (address < 0x4000)
        value = ppu_->Read(address);
    else if (address < 0x4020)
    {
        if (address == 0x4016)
            value = -controller_->Read();
        else
            value = apu_->Read(address);
    }
    else if (cart_)
        value = cart_->CpuRead(address);
    else
        value = 0;

    TickCpu();

    return value;
}

uint8_t Bus::CpuReadProgramData(uint16_t address)
{
    uint8_t value;

    // program data most likely comes from cartridge
    if (address > 0x4020)
    {
        if (cart_)
            value = cart_->CpuRead(address);
        else
            value = 0;
    }
    else if (address < 0x2000)
        value = cpuRam_[address & 0x7ff];
    else if (address < 0x4000)
        value = ppu_->Read(address);
    else if (address < 0x4020)
    {
        if (address == 0x4016)
            value = controller_->Read();
        else
            value = apu_->Read(address);
    }
    else
        value = 0;

    TickCpu();

    return value;
}

void Bus::CpuWrite(uint16_t address, uint8_t value)
{
    TickCpu();

    if (address < 0x2000)
        cpuRam_[address & 0x7ff] = value;
    else if (address < 0x4000)
        ppu_->Write(address, value);
    else if (address < 0x4020)
    {
        if (address == 0x4014)
            RunDma(value);
        else if (address == 0x4016)
            controller_->Write(value);
        else
            apu_->Write(address, value);
    }
    else if (address >= 0x6000)
    {
        cart_->CpuWrite(address, value);
    }
}

void Bus::CpuWrite2(uint16_t address, uint8_t firstValue, uint8_t secondValue)
{
    TickCpu();

    if (address < 0x2000)
    {
        TickCpu();
        cpuRam_[address & 0x7ff] = secondValue;
    }
    else if (address < 0x4000)
    {
        ppu_->Write(address, firstValue);
        TickCpu();
        ppu_->Write(address, secondValue);
    }
    else if (address < 0x4020)
    {
        if (address == 0x4014)
        {
            RunDma(firstValue);
            TickCpu();
            RunDma(secondValue);
        }
        else if (address == 0x4016)
        {
            controller_->Write(firstValue);
            TickCpu();
            controller_->Write(secondValue);
        }
        else
        {
            apu_->Write(address, firstValue);
            TickCpu();
            apu_->Write(address, secondValue);
        }
    }
    else if (address >= 0x6000)
    {
        cart_->CpuWrite2(address, firstValue, secondValue);
    }
}

uint8_t Bus::PpuRead(uint16_t address) const
{
    if (address < 0x2000)
        return cart_->PpuRead(address);

    address = cart_->EffectivePpuRamAddress(address);
    return ppuRam_[address];
}

uint16_t Bus::PpuReadChr16(uint16_t address) const
{
    return cart_->PpuReadChr16(address);
}

void Bus::PpuWrite(uint16_t address, uint8_t value)
{
    if (address >= 0x2000)
    {
        address = cart_->EffectivePpuRamAddress(address);
        ppuRam_[address] = value;
    }
    else
    {
        cart_->PpuWrite(address, value);
    }
}

void Bus::SignalNmi()
{
    cpu_->SignalNmi();
}

void Bus::DmaWrite(uint8_t value)
{
    TickCpu();
    ppu_->DmaWrite(value);
}

void Bus::OnFrame()
{
    apu_->SyncFrame();
}

void Bus::RunDma(uint8_t page)
{
    TickCpu();

    if (CycleCount() & 1)
    {
        TickCpu();
    }

    auto address = (uint16_t)(page << 8);
    auto endAddress = (uint16_t)(address + 0x100);
    while (address != endAddress)
    {
        auto value = CpuReadData(address++);
        DmaWrite(value);
    }
}
