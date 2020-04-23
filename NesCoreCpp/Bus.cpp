#include "Bus.h"

#include "Cart.h"
#include "Controller.h"
#include "Cpu.h"
#include "Ppu.h"


Bus::Bus() :
    cpu_(nullptr),
    ppu_(nullptr),
    apu_(nullptr),
    controller_(nullptr)
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
}

void Bus::TickCpu()
{
    apu_->Tick();
    ppu_->Tick3();
}

uint8_t Bus::CpuReadData(uint16_t address)
{
    if (address < 0x2000)
        return cpuRam_[address & 0x7ff];

    if (address < 0x4000)
        return ppu_->Read(address);

    if (address < 0x4020)
    {
        if (address == 0x4016)
            return controller_->Read();

        return 0;
    }

    if (cart_)
        return cart_->CpuRead(address);

    return 0;
}

uint8_t Bus::CpuReadProgramData(uint16_t address)
{
    // program data most likely comes from cartridge
    if (address > 0x4020)
    {
        if (cart_)
        {
            return cart_->CpuRead(address);
        }

        return 0;
    }

    if (address < 0x2000)
        return cpuRam_[address & 0x7ff];

    if (address < 0x4000)
        return ppu_->Read(address);

    if (address < 0x4020)
    {
        if (address == 0x4016)
            return controller_->Read();
    }

    return 0;
}

void Bus::CpuWrite(uint16_t address, uint8_t value)
{
    if (address < 0x2000)
        cpuRam_[address & 0x7ff] = value;
    else if (address < 0x4000)
        ppu_->Write(address, value);
    else if (address < 0x4020)
    {
        if (address == 0x4014)
            cpu_->RunDma(value);
        else if (address == 0x4016)
            controller_->Write(value);
        else
            apu_->Write(address, value);
    }
    else if (address >= 0x6000)
    {
        // TODO: we only need to sync when we change a PPU bank
        ppu_->Sync();
        cart_->CpuWrite(address, value);
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
    ppu_->DmaWrite(value);
}

void Bus::OnFrame()
{
    apu_->SyncFrame();
}
