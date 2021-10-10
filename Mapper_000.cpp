// Copyright 2018, 2019, 2020, 2021 OneLoneCoder.com
#include "Mapper_000.h"


Mapper000::Mapper000(uint8_t prgBanks, uint8_t chrBanks) : Mapper(prgBanks, chrBanks){}
Mapper000::~Mapper000(){}

void Mapper000::reset(){}


bool Mapper000::mapperCpuRead(uint16_t addr, uint32_t &mappedAddr)
{
    if (addr >= 0x8000 && addr <= 0xFFFF)
	{
		//mappedAddr = addr & (nPRGBanks > 1 ? 0x7FFF : 0x3FFF);
		if (nPRGBanks > 1)
		{
			mappedAddr = addr & 0x7FFF;
		}
		else
		{
			mappedAddr = addr & 0x3FFF;
		};
		return true;
	};

	return false;
}

bool Mapper000::mapperCpuWrite(uint16_t addr, uint32_t &mappedAddr, uint8_t data)
{
	if (addr >= 0x8000 && addr <= 0xFFFF)
	{
		//mappedAddr = addr & (nPRGBanks > 1 ? 0x7FFF : 0x3FFF);
		if (nPRGBanks > 1)
		{
			mappedAddr = addr & 0x7FFF;
		}
		else
		{
			mappedAddr = addr & 0x3FFF;
		};
		return true;
	};

	return false;
}

bool Mapper000::mapperPpuRead(uint16_t addr, uint32_t &mappedAddr)
{
	// no mapping required for PPU
	if (addr >= 0x0000 && addr <= 0x1FFF)
	{
		mappedAddr = addr;
		return true;
	};

	return false;
}

bool Mapper000::mapperPpuWrite(uint16_t addr, uint32_t &mappedAddr)
{
	if (addr >= 0x0000 && addr <= 0x1FFF)
	{
		if (nCHRBanks == 0)
		{
			// Treat as RAM
			mappedAddr = addr;
			return true;
		};
	};

	return false;
}