#pragma once
#include "Mapper.h"

class Mapper000 : public Mapper
{
    public:
        Mapper000(uint8_t PRG_size, uint8_t CHR_size);
        ~Mapper000();
    
    public:
        // Communication with Bus
        bool mapperCpuRead(uint16_t addr, uint32_t &mappedAddr) override;
        bool mapperCpuWrite(uint16_t addr,  uint32_t &mappedAddr, uint8_t data = 0) override;

        // Communication with PPU Bus
        bool mapperPpuRead(uint16_t addr, uint32_t &mappedAddr) override;
        bool mapperPpuWrite(uint16_t addr,  uint32_t &mappedAddr) override;

        void reset() override;
    
};