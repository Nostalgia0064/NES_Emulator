#pragma once
#include <stdio.h>
#include <array>
#include <string>
#include <vector>
#include <stdint.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>

class Mapper
{
    public:
        Mapper(uint8_t PRGsize, uint8_t CHRsize);
        ~Mapper();

    public:
        // Communication with Bus
        virtual bool mapperCpuRead(uint16_t addr, uint32_t &mappedAddr) = 0;
        virtual bool mapperCpuWrite(uint16_t addr,  uint32_t &mappedAddr, uint8_t data) = 0;

        // Communication with PPU Bus
        virtual bool mapperPpuRead(uint16_t addr, uint32_t &mappedAddr) = 0;
        virtual bool mapperPpuWrite(uint16_t addr,  uint32_t &mappedAddr) = 0;

        virtual void reset() = 0;

    protected:
        uint8_t nPRGBanks;
        uint8_t nCHRBanks;
};
