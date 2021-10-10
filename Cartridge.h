#pragma once
#include <stdio.h>
#include <array>
#include <string>
#include <vector>
#include <stdint.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdint>
#include <memory>

#include "Mapper_000.h"

class Cartridge
{
    public:
        // Constructors
        Cartridge(const std::string &FileName);
        ~Cartridge();

        // Communication with Bus
        bool cpuRead(uint16_t addr, uint8_t &data);
        bool cpuWrite(uint16_t addr,  uint8_t data);

        // Communication with PPU Bus
        bool ppuRead(uint16_t addr, uint8_t &data);
        bool ppuWrite(uint16_t addr,  uint8_t data);

        bool ImageValid();
        void reset();

        enum MIRROR
        {
            HORIZONTAL,
            VERTICAL,
            ONESCREEN_LO,
            ONESCREEN_HI,
            
        } mirror = HORIZONTAL;

        bool imageValid;

    private:
        uint8_t nMapperID = 0;
	    uint8_t nPRGBanks;
	    uint8_t nCHRBanks;
        uint8_t file_format;

        std::shared_ptr<Mapper> pMapper;
        std::vector<uint8_t> PRG_Memory;
        std::vector<uint8_t> CHR_Memory;


            
        

};
