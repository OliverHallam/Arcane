#include "Bus.h"

#include "Cart.h"
#include "Controller.h"
#include "Cpu.h"
#include "Ppu.h"


Bus::Bus()
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
    ppu_->Tick();
    ppu_->Tick();
    ppu_->Tick();
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
    }
}

uint8_t Bus::PpuRead(uint16_t address)
{
    if (address < 0x2000)
        return cart_->PpuRead(address);

    if (cart_->VerticalMirroring())
    {
        address &= 0x07ff;
    }
    else
    {
        address &= 0x0bff;
        if (address >= 0x0800)
            address -= 0x0400;
    }

    return ppuRam_[address];
}

void Bus::PpuWrite(uint16_t address, uint8_t value)
{
    if (address >= 0x2000)
    {
        if (cart_->VerticalMirroring())
        {
            address &= 0x07ff;
        }
        else
        {
            address &= 0x0bff;
            if (address >= 0x0800)
                address -= 0x0400;
        }

        ppuRam_[address] = value;
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
