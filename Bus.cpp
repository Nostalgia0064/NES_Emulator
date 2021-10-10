// Copyright 2018, 2019, 2020, 2021 OneLoneCoder.com
#include "Bus.h"

Bus::Bus() 
{
    // Connect CPU -> Bus
    cpu.ConnectBus(this);
}

Bus::~Bus(){}

void Bus::write(uint16_t addr, uint8_t data)
{
    // Set Bus contents for debugging modules
    bus_addr = addr;
    bus_data = data;

    // Cartridge Override
    if(cart->cpuWrite(addr, data))
    {
        bus_accessType = "Cartridge Write";
    }
    // Cpu Addressable Range
    else if(addr >= 0x0000 && addr <= 0x1FFF)
    {
        cpuRam[addr & 0x07FF] = data;   // Mirror every 2KB's
        bus_accessType = "Cpu Write";
    }
    // PPU Mapped Register Range
    else if (addr >= 0x2000 && addr <= 0x3FFF)
    {
        ppu.cpuWrite(addr & 0x0007, data);
        bus_accessType = "Ppu Write";
    } 
    // Controller 1 Mapped Range
    else if(addr == 0x4016)
    {
        // Poll Input
        //if (data == 0x01)
        //{
            controllerState[0] = controllers[0];
            bus_accessType = "Controller Write";
        //};
        //else if(data == 0x00)
        //{

        //};

    };
}


uint8_t Bus::read(uint16_t addr, bool bReadOnly)
{
    uint8_t data = 0x00;

    // Set Bus contents for debugging modules
    bus_addr = addr;

    // Cartridge Override
    if(cart->cpuRead(addr, data))
    {
       bus_accessType = "Cartridge Read";
    }
    // Cpu Addressable Range
    else if(addr >= 0x0000 && addr <= 0x1FFF)
    {
        data = cpuRam[addr & 0x07FF];   // Mirror every 2KB's
        bus_accessType = "Cpu Read";
    } 
    // PPU Mapped Register Range
    else if (addr >= 0x2000 && addr <= 0x3FFF)
    {
        data = ppu.cpuRead(addr & 0x0007);
        bus_accessType = "Ppu Read";
    }
    // Controller 1 Mapped Addr
    else if (addr == 0x4016)
    {
        if((controllerState[0] & 0x80) > 0)
        {
            data = 0x01;
        }
        else
        {
            data = 0x00;
        }; 
        controllerState[0] <<= 1;
        bus_accessType = "Controller Read";
    };
    bus_data = data;

    return data;
}

void Bus::setController(uint8_t bits, uint8_t num)
{
    num &= 0x01;
    controllers[num] = bits;
}

uint8_t Bus::getController(uint8_t num)
{
    num &= 0x01;
    controllers[num] = userInput;
    return controllers[num];
}


void Bus::insertCartridge(const std::shared_ptr<Cartridge> &cartridge)
{
    this->cart = cartridge;
    ppu.connectCartridge(cartridge);
}

void Bus::systemReset()
{
    cart->reset();
    cpu.reset();
    ppu.reset();
    NES_SystemClock = 0;
    
}

void Bus::systemClock()
{
    ppu.render();

    if ((NES_SystemClock % 3) == 0)
    {
        cpu.execute();
    };

    if(ppu.NMI == true)
    {
        cpu.nmi();
        ppu.NMI = false;
    };

    NES_SystemClock++;
}