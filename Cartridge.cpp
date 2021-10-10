// Copyright 2018, 2019, 2020, 2021 OneLoneCoder.com
#include "Cartridge.h"

Cartridge::Cartridge(const std::string &FileName)
{

    struct FileHeader
    {
        char constant[4];
        uint8_t prg_rom_chunks;
		uint8_t chr_rom_chunks;
        uint8_t mapper;
        uint8_t mapper2_0;
        uint8_t prg_ram_size;
        uint8_t TV_system;
        uint8_t prg_ram_pres;
        char padding[5];
    } header;

    std::ifstream file;
    imageValid = false;

    // Open ROM file
    file.open(FileName, std::ifstream::binary);
    if(file.is_open())
    {
        // Read File Header
        file.read((char*) &header, sizeof(FileHeader));

        // Read Trainer Data
        if(header.mapper & 0x04)
        {
            //file.read((char*) &ines.trainer, sizeof(ines.trainer));
            file.seekg(512, std::ios_base::cur);
        };

        // Get Mapper ID from Header
        nMapperID = ((header.mapper2_0 >> 4) << 4) | (header.mapper >> 4);
        
        // Get Mirror type from Header
        //mirror = (header.mapper & 0x01) ? VERTICAL : HORIZONTAL;

        if(header.mapper & 0x01) 
        {
            mirror = VERTICAL;
        }
        else
        {
            mirror = HORIZONTAL;
        };
        // For Donkey Kong Only (DEBUGGING/ DELETE LATER)
        //mirror = VERTICAL;

        uint8_t fileType = 1;

        // Determine .nes file format
        if (fileType = 0) 
        {
            // iNES 2.0
            file_format = 0x00;
        }

        if (fileType = 1)
        //else if((ines.header.mapper2_0 & 0x0C) == 0x00 || (fileType == 1))
        {
            // iNES
            file_format = 0x01;

            nPRGBanks = header.prg_rom_chunks;
            // Set size of PRG & CHR data
            PRG_Memory.resize((nPRGBanks) * 16384);
            file.read((char*)PRG_Memory.data(), PRG_Memory.size());

            nCHRBanks = header.chr_rom_chunks;
            if (nCHRBanks == 0)
            {
                // Create CHR RAM
                CHR_Memory.resize(8192);
            }
            else
            {
                // Allocate for ROM
                CHR_Memory.resize(nCHRBanks * 8192);
            };
            file.read((char*)CHR_Memory.data(), CHR_Memory.size());
        }
        else
        {
            // Archaic iNES
            file_format = 0x02;
        };

        switch(nMapperID)
        {
            case 0:
                pMapper = std::make_shared<Mapper000>(nPRGBanks, nCHRBanks);
                break;

            case 1:
                break;

            case 2:
                break;
        };
        imageValid = true;
        file.close();
    };
}

Cartridge::~Cartridge(){}

bool Cartridge::ImageValid()
{
	return imageValid;
}


void Cartridge::reset()
{
    if (pMapper != nullptr)
		pMapper->reset();
}

bool Cartridge::cpuRead(uint16_t addr, uint8_t &data)
{
    uint32_t mappedAddr = 0;

    // Read Mapped address from specific Mapper
    if(pMapper->mapperCpuRead(addr, mappedAddr))
    {
        data = PRG_Memory[mappedAddr];
        return true;
    }
    else
    {
        return false;
    };
}

bool Cartridge::cpuWrite(uint16_t addr,  uint8_t data)
{
    uint32_t mappedAddr = 0;

    // Write Mapped address from specific Mapper
    if(pMapper->mapperCpuWrite(addr, mappedAddr, data))
    {
        PRG_Memory[mappedAddr] = data;
        return true;
    }
    else
    {
        return false;
    };
}

bool Cartridge::ppuRead(uint16_t addr, uint8_t &data)
{
    uint32_t mappedAddr = 0;

    // Read Mapped address from specific Mapper
    if(pMapper->mapperPpuRead(addr, mappedAddr))
    {
        data = CHR_Memory[mappedAddr];
        return true;
    }
    else
    {
        return false;
    };
}

bool Cartridge::ppuWrite(uint16_t addr,  uint8_t data)
{
    uint32_t mappedAddr = 0;

    // Write Mapped address from specific Mapper
    if(pMapper->mapperPpuWrite(addr, mappedAddr))
    {
        CHR_Memory[mappedAddr] = data;
        return true;
    }
    else
    {
        return false;
    };
}
