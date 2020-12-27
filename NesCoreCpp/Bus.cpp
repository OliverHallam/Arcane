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
    cart_(nullptr)
{
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

void Bus::Attach(Cart* cart)
{
    cart_ = std::move(cart);
    if (cart_)
    {
        cart_->Attach(this);

        if (cart_->UsesMMC5Audio())
            apu_->EnableMMC5(true);
    }
}

void Bus::DetachCart()
{
    if (!cart_)
        return;

    if (cart_->UsesMMC5Audio())
        apu_->EnableMMC5(false);

    cart_ = nullptr;
}

bool Bus::HasCart() const
{
    return !!cart_;
}

void Bus::TickCpuRead()
{
    if (state_.Dma)
        RunDma();

    Tick();
}

void Bus::TickCpuWrite()
{
    Tick();
}

uint32_t Bus::CpuCycleCount() const
{
    return state_.CycleCount / 3;
}

uint32_t Bus::PpuCycleCount() const
{
    return state_.CycleCount;
}

void Bus::SyncPpu()
{
    ppu_->Sync();
}

ChrA12Sensitivity Bus::ChrA12Sensitivity() const
{
    return cart_->ChrA12Sensitivity();
}

void Bus::UpdateA12Sensitivity()
{
    return ppu_->UpdateA12Sensitivity(false);
}

uint32_t Bus::GetChrA12PulsesRequiredForSync() const
{
    return cart_->A12PulsesUntilSync();
} 
void Bus::ChrA12Rising()
{
#ifdef DIAGNOSTIC
    ppu_->MarkDiagnostic(0XFF0080FF);
#endif
    cart_->ChrA12Rising();
}

void Bus::ChrA12Falling()
{
#ifdef DIAGNOSTIC
    ppu_->MarkDiagnostic(0XFF00FFFF);
#endif
    cart_->ChrA12Falling();
}

bool Bus::HasScanlineCounter() const
{
    return cart_->HasScanlineCounter();
}

void Bus::ScanlineCounterBeginScanline()
{
    cart_->ScanlineCounterBeginScanline();
}

void Bus::TileSplitBeginScanline(bool firstTileIsAttribute)
{
    cart_->TileSplitBeginScanline(firstTileIsAttribute);
}

void Bus::CpuDummyRead(uint16_t address)
{
    // TODO: side effects
    TickCpuRead();
}

uint8_t Bus::CpuReadData(uint16_t address)
{
    TickCpuRead();

    if (address < 0x2000)
        return state_.CpuRam[address & 0x7ff];
    else if (address < 0x4020)
    {
        if (address < 0x4000)
            return ppu_->Read(address);
        else if (address == 0x4016)
            return -controller_->Read();
        else
            return apu_->Read(address);
    }
    else if (cart_)
        return cart_->CpuRead(address);
    else
        return 0;
}

uint8_t Bus::CpuReadZeroPage(uint16_t address)
{
    TickCpuRead();
    return state_.CpuRam[address];
}

uint8_t Bus::CpuReadProgramData(uint16_t address)
{
    TickCpuRead();

    // program data most likely comes from cartridge
    if (address > 0x4020)
    {
        if (cart_)
            return cart_->CpuRead(address);
        else
            return 0;
    }
    else
    {
        return CpuReadProgramDataRare(address);
    }
}

void Bus::CpuWrite(uint16_t address, uint8_t value)
{
    TickCpuWrite();

    if (address < 0x2000)
        state_.CpuRam[address & 0x7ff] = value;
    else if (address < 0x4000)
        ppu_->Write(address, value);
    else if (address < 0x4020)
    {
        if (address == 0x4014)
            BeginOamDma(value);
        else if (address == 0x4016)
            controller_->Write(value);
        else
            apu_->Write(address, value);
    }
    else
    {
        cart_->CpuWrite(address, value);
    }
}

void Bus::CpuWriteZeroPage(uint16_t address, uint8_t value)
{
    TickCpuWrite();

    state_.CpuRam[address] = value;
}

void Bus::CpuWrite2(uint16_t address, uint8_t firstValue, uint8_t secondValue)
{
    TickCpuWrite();

    if (address < 0x2000)
    {
        Tick();
        state_.CpuRam[address & 0x7ff] = secondValue;
    }
    else if (address < 0x4000)
    {
        ppu_->Write(address, firstValue);
        Tick();
        ppu_->Write(address, secondValue);
    }
    else if (address < 0x4020)
    {
        if (address == 0x4014)
        {
            BeginOamDma(firstValue);
            Tick();
        }
        else if (address == 0x4016)
        {
            controller_->Write(firstValue);
            Tick();
            controller_->Write(secondValue);
        }
        else
        {
            apu_->Write(address, firstValue);
            Tick();
            apu_->Write(address, secondValue);
        }
    }
    else
    {
        cart_->CpuWrite2(address, firstValue, secondValue);
    }
}

uint8_t* Bus::GetPpuRamBase()
{
    return &state_.PpuRam[0];
}

uint8_t Bus::PpuRead(uint16_t address) const
{
    return cart_->PpuRead(address);
}

uint16_t Bus::PpuReadChr16(uint16_t address) const
{
    return cart_->PpuReadChr16(address);
}

void Bus::PpuDummyTileFetch() const
{
    cart_->PpuDummyTileFetch();
}

void Bus::PpuDummyNametableFetch() const
{
    cart_->PpuSpriteNametableFetch();
}

void Bus::PpuWrite(uint16_t address, uint8_t value)
{
    cart_->PpuWrite(address, value);
}

void Bus::InterceptPpuCtrl(bool largeSprites)
{
    cart_->InterceptWritePpuCtrl(largeSprites);
}

void Bus::InterceptPpuMask(bool renderingEnabled)
{
    cart_->InterceptWritePpuMask(renderingEnabled);
}

void Bus::WriteMMC5Audio(uint16_t address, uint8_t value)
{
    apu_->WriteMMC5(address, value);
}

void Bus::SignalNmi()
{
    // There a 2 cycle delay to the NMI triggering
    SchedulePpu(2, SyncEvent::CpuNmi);
}

void Bus::SetAudioIrq(bool irq)
{
    if (irq != state_.AudioIrq)
    {
        state_.AudioIrq = irq;

        if (!state_.CartIrq)
            SchedulePpu(2, irq ? SyncEvent::CpuSetIrq : SyncEvent::CpuClearIrq);
    }
}

void Bus::SetCartIrq(bool irq)
{
    if (irq != state_.CartIrq)
    {
        state_.CartIrq = irq;

        if (!state_.AudioIrq)
            SchedulePpu(2, irq ? SyncEvent::CpuSetIrq : SyncEvent::CpuClearIrq);
    }
}

uint8_t Bus::DmcDmaRead(uint16_t address)
{
    uint8_t value;
    if (cart_)
        value = cart_->CpuRead(address);
    else
        value = 0;

    Tick();
    return value;
}

void Bus::OamDmaWrite(uint8_t value)
{
    ppu_->DmaWrite(value);
    Tick();
}

void Bus::BeginOamDma(uint8_t page)
{
    state_.OamDmaAddress = (uint16_t)(page << 8);
    state_.OamDma = true;
    state_.Dma = true;
}

void Bus::BeginDmcDma(uint16_t address)
{
    state_.DmcDmaAddress = address;
    state_.DmcDma = true;
    state_.Dma = true;
}

void Bus::OnFrame()
{
    apu_->SyncFrame();
}

void Bus::Schedule(uint32_t cycles, SyncEvent evt)
{
    state_.SyncQueue.Schedule(state_.CycleCount + cycles * 3, evt);
}

void Bus::SchedulePpu(uint32_t cycles, SyncEvent evt)
{
    state_.SyncQueue.Schedule(state_.CycleCount + cycles, evt);
}

bool Bus::Deschedule(SyncEvent evt)
{
    return state_.SyncQueue.Deschedule(evt);
}

bool Bus::DescheduleAll(SyncEvent evt)
{
    return state_.SyncQueue.DescheduleAll(evt);
}

void Bus::CaptureState(BusState* state) const
{
    *state = state_;
}

void Bus::RestoreState(const BusState& state)
{
    state_ = state;
}

#ifdef DIAGNOSTIC

void Bus::MarkDiagnostic(uint32_t color)
{
    ppu_->MarkDiagnostic(color);
}

#endif

void Bus::Tick()
{
    auto nextCycleCount = state_.CycleCount + 3;

    // there is always at least one event scheduled so we can skip the check that the queue is empty
    uint32_t nextEventTime;
    while ((nextEventTime = state_.SyncQueue.GetNextEventTime()) <= nextCycleCount)
    {
        state_.CycleCount = nextEventTime;
        RunEvent();
    }

    state_.CycleCount = nextCycleCount;
}

uint8_t Bus::CpuReadProgramDataRare(uint16_t address)
{
    if (address < 0x2000)
        return state_.CpuRam[address & 0x7ff];
    else if (address < 0x4000)
        return ppu_->Read(address);
    else if (address < 0x4020)
    {
        if (address == 0x4016)
            return controller_->Read();
        else
            return apu_->Read(address);
    }
    else
        return 0;
}

void Bus::RunEvent()
{
    switch (state_.SyncQueue.PopEvent())
    {
    case SyncEvent::ApuSample:
        apu_->Sample();
        break;

    case SyncEvent::ApuSync:
        apu_->Sync();
        break;

    case SyncEvent::ApuFrameCounter:
        apu_->ActivateFrameCounter();
        break;

    case SyncEvent::PpuStateUpdate:
        ppu_->SyncState();
        break;

    case SyncEvent::PpuScanline:
        ppu_->SyncScanline();
        break;

    case SyncEvent::PpuSyncA12:
        ppu_->SyncA12();
        break;

    case SyncEvent::PpuSync:
        ppu_->Sync();
        break;

    case SyncEvent::ScanlineCounterScanline:
        cart_->ScanlineCounterBeginScanline();
        break;

    case SyncEvent::ScanlineCounterEndFrame:
        cart_->ScanlineCounterEndFrame();
        break;

    case SyncEvent::CpuNmi:
        cpu_->Nmi();
        break;

    case SyncEvent::CpuSetIrq:
#if DIAGNOSTIC
        ppu_->MarkDiagnostic(0xff00ff00);
#endif
        cpu_->SetIrq(true);
        break;

    case SyncEvent::CpuClearIrq:
        cpu_->SetIrq(false);
    }
}

void Bus::RunDma()
{
    if (state_.OamDma)
    {
        RunOamDma();
        return;
    }

    if (state_.DmcDma)
    {
        RunDmcDma();
        return;
    }
}

void Bus::RunOamDma()
{
    state_.OamDma = false;
    state_.Dma = false;

    bool dmcStarted = state_.DmcDma;

    // TODO: phantom read.
    Tick();

    if (CpuCycleCount() & 1)
    {
        // TODO: phantom read.
        Tick();
    }

    auto address = state_.OamDmaAddress;
    auto endAddress = (uint16_t)(address + 0x100);

    while (address != endAddress)
    {
        if (dmcStarted)
        {
            auto value = DmcDmaRead(state_.DmcDmaAddress);
            apu_->SetDmcBuffer(value);

            // dummy read
            Tick();

            state_.DmcDma = false;
            state_.Dma = false;
        }

        // it takes two cycles for the dmc to kick in
        dmcStarted = state_.DmcDma;

        auto value = CpuReadData(address++);
        OamDmaWrite(value);
    }

    if (dmcStarted)
    {
        auto value = DmcDmaRead(state_.DmcDmaAddress);
        apu_->SetDmcBuffer(value);

        state_.DmcDma = false;
        state_.Dma = false;
    }
}

void Bus::RunDmcDma()
{
    state_.DmcDma = false;
    state_.Dma = false;

    // TODO: phantom read.
    Tick();
    Tick();

    auto value = DmcDmaRead(state_.DmcDmaAddress);
    apu_->SetDmcBuffer(value);
}
