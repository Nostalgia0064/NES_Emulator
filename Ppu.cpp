// Copyright 2018, 2019, 2020, 2021 OneLoneCoder.com

#include "Ppu.h"

Ppu::Ppu()
{
    // Set pixels to RGB color engine
    tbl.colors = {

        // Row 1
        {84, 84, 84}, {0, 30, 116}, {8, 16, 144}, {48, 0, 136}, {68, 0, 100}, {92, 0, 48}, {84, 4, 0},{60, 24, 0},
        {32, 42, 0}, {8, 58, 0}, {0, 64, 0}, {0, 60, 0}, {0, 50, 60}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0},

        // Row 2
        {152, 150, 152}, {8, 76, 196}, {48, 50, 236}, {92, 30, 228}, {136, 20, 176}, {160, 20, 100}, {152, 34, 32}, {120, 60, 0},
        {84, 90, 0}, {40, 114, 0}, {8, 124, 0}, {0, 118, 40}, {0, 102, 120}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0},

        // Row 3
        {236, 238, 236}, {76, 154, 236}, {120, 124, 236}, {176, 98, 236}, {228, 84, 236}, {236, 88, 180}, {236, 106, 100}, {212, 136, 32},
        {160, 170, 0}, {116, 196, 0}, {76, 208, 32}, {56, 204, 108}, {56, 180, 204}, {60, 60, 60}, {0, 0, 0}, {0, 0, 0},

        // Row 4
        {236, 238, 236}, {168, 204, 236}, {188, 188, 236}, {212, 178, 236}, {236, 174, 236}, {236, 174, 212}, {236, 180, 176}, {228, 196, 144},
        {204, 210, 120}, {180, 222, 120}, {168, 226, 144}, {152, 226, 180}, {160, 214, 228}, {160, 162, 160}, {0, 0, 0}, {0, 0, 0}   

    };

    memset(screenPixels, 0, sizeof(screenPixels)); 
}

Ppu::~Ppu(){}

void Ppu::connectCartridge(const std::shared_ptr<Cartridge>& cartridge)
{
    this->cart = cartridge;
}

void Ppu::reset()
{
    r.controller = 0x00;
    r.mask = 0x00;
    r.status = 0x00;
    r.oam_addr = 0x00;
    r.scroll = 0x00;
    r.ppu_addr = 0x00;
    r.ppu_data = 0x00;
    cycles = 0;
    scanLine = 0;
    buffer = 0x00;

    fine_X = 0x00;
    toggle = 0x00;
    currVRAM = 0x0000;
    tempVRAM = 0x0000;

    fetched.patternLeft = 0x00;
    fetched.patternRight = 0x00;
    fetched.tileAttr = 0x00;
    fetched.tileNameTbl = 0x00;

    shift.palAttribHi = 0x0000;
    shift.palAttribLow = 0x0000;
    shift.patternLeft = 0x0000;
    shift.patternRight = 0x0000;

}

std::vector<Ppu::Pixels> Ppu::getPatternTables(int plane, uint8_t pal)
{
    int i = 0;
    patterns.clear(); 

    for (int tileY = 0; tileY < 16; tileY++)
    {
        for(int tileX = 0; tileX < 16; tileX++) 
        {
            uint16_t offset = tileY * 256 + (tileX * 16);

            for (int row = 0; row < 8; row++)
            {
                uint8_t tile_lsb = ppuRead(plane * 0x1000 + offset + row + 0x0000);
				uint8_t tile_msb = ppuRead(plane * 0x1000 + offset + row + 0x0008);

                for (int col = 0; col < 8; col++)
                {
                    uint8_t pixel = (tile_lsb & 0x01) << 1 | (tile_msb & 0x01);
                    tile_lsb >>= 1;
                    tile_msb >>= 1;

                    // GET PIXEL COLOR (R,G,B)
                    uint8_t palAddr = getPixelColorIndex(pal, pixel);

                    // Get RGB values from color palette
                    uint8_t red = tbl.colors[palAddr].red;
                    uint8_t green = tbl.colors[palAddr].green;
                    uint8_t blue = tbl.colors[palAddr].blue;
                    uint8_t x = tileX * 8 + (7 - col);
                    uint8_t y = tileY * 8 + row;

                    // Render Pixel to Screen through Game Engine
                    Pixels elem = {red, green, blue, x, y};
                    patterns.push_back(elem);
                
                };
            };
        };
    };
    return patterns;
}

inline uint8_t Ppu::getPixelColorIndex(uint8_t &palette, uint8_t &pattern)
{
    palAddr = (ppuRead(0x3F00 + (palette << 2) + pattern)) & 0x3F;
    //palAddr = palAddr & 0x3F;

    return palAddr;
}

Ppu::Pixels Ppu::getScreenPixels()
{
    return renderPixel;
}

std::vector<Ppu::RGB> Ppu::getPalettes()
{
    int length = (int) sizeof(tbl.palettesMem);
    RGB palpix;
    palettes.clear();

    for (int i = 0; i < length; i++)
    {   
        palpix.red = tbl.colors[tbl.palettesMem[i]].red;
        palpix.green = tbl.colors[tbl.palettesMem[i]].green;
        palpix.blue = tbl.colors[tbl.palettesMem[i]].blue;
        palettes.push_back(palpix);
    };

    return palettes;
}

// ---------------------------- BUSES ---------------------------- //
inline uint8_t Ppu::ppuRead(uint16_t addr)
{
    // Read to PPU bus
    uint8_t data = 0x00;
    addr &= 0x3FFF;
       
    if (cart->ppuRead(addr, data))
    {

    }
    else
    {
        // Pattern Tables
        if (addr >= 0x0000 && addr <= 0x1FFF)
        {
            data = tbl.patterns[((addr & 0x1000) & 0x2000) > 0][addr & 0x0FFF];
        }

        // Name Tables & mirroring
        else if (addr >= 0x2000 && addr <= 0x3EFF)
        {
            addr &= 0x0FFF;

            // Vertical Mirror
            if(cart->mirror == Cartridge::MIRROR::VERTICAL)
            {
                if (addr >= 0x0000 && addr <= 0x03FF)
                {
                    data = tbl.name[0][addr & 0x03FF];
                };         
                if (addr >= 0x0400 && addr <= 0x07FF)
                {
                    data = tbl.name[1][addr & 0x03FF];
                };
                if (addr >= 0x0800 && addr <= 0x0BFF)
                {
                    data = tbl.name[0][addr & 0x03FF];
                };
                if (addr >= 0x0C00 && addr <= 0x0FFF)
                {
                    data = tbl.name[1][addr & 0x03FF];
                };

            }
            // Horizontal Mirror
            else if (cart->mirror == Cartridge::MIRROR::HORIZONTAL)
            {

                if (addr >= 0x0000 && addr <= 0x03FF)
                {
                    data = tbl.name[0][addr & 0x03FF];   
                };                   
                        
                if (addr >= 0x0400 && addr <= 0x07FF)
                {
                    data = tbl.name[0][addr & 0x03FF];   
                };                    

                if (addr >= 0x0800 && addr <= 0x0BFF)
                {
                    data = tbl.name[1][addr & 0x03FF];
                };

                if (addr >= 0x0C00 && addr <= 0x0FFF)
                {
                    data = tbl.name[1][addr & 0x03FF];
                };
            }
        }
            
        // Palettes
        else if (addr >= 0x3F00 && addr <= 0x3FFF)
        {
            addr &= 0x001F;

            if (addr == 0x0010)
            {
                addr = 0x0000;
            };

            if (addr == 0x0014)
            {
                addr = 0x0004;
            };

            if (addr == 0x0018)
            {
                addr = 0x0008;
            };

            if (addr == 0x001C)
            {
                addr = 0x000C;
            };

            if(getMaskBits(G))
            {
                data = tbl.palettesMem[addr] & 0x30;
            }
            else
            {
                data = tbl.palettesMem[addr] & 0x3F;
            };

        };
        
    };
    return data;
}

inline void Ppu::ppuWrite(uint16_t addr, uint8_t data)
{
    addr &= 0x3FFF;
    if (cart->ppuWrite(addr, data))
    {
        
    }
    else
    {
        switch (addr)
        {   
            // Pattern Tables
            case 0x0000 ... 0x1FFF:
                tbl.patterns[((addr & 0x1000) & 0x2000) > 0][addr & 0x0FFF] = data;
                break;

            // Name Tables
            case 0x2000 ... 0x3EFF:
                addr &= 0x0FFF;

                // Vertical Mirror
                if(cart->mirror == Cartridge::MIRROR::VERTICAL)
                {
                    switch (addr)
                    {
                        case 0x0000 ... 0x03FF:
                            tbl.name[0][addr & 0x03FF] = data;
                            break;
                            
                        case 0x0400 ... 0x07FF:
                            tbl.name[1][addr & 0x03FF] = data;
                            break;

                        case 0x0800 ... 0x0BFF:
                            tbl.name[0][addr & 0x03FF] = data;
                            break;

                        case 0x0C00 ... 0x0FFF:
                            tbl.name[1][addr & 0x03FF] = data;
                            break;
                    };
                }
                // Horizontal Mirror
                else if (cart->mirror == Cartridge::MIRROR::HORIZONTAL)
                {
                    switch (addr)
                    {
                        case 0x0000 ... 0x03FF:
                            tbl.name[0][addr & 0x03FF] = data;
                            break;
                            
                        case 0x0400 ... 0x07FF:
                            tbl.name[0][addr & 0x03FF] = data;
                            break;

                        case 0x0800 ... 0x0BFF:
                            tbl.name[1][addr & 0x03FF] = data;
                            break;

                        case 0x0C00 ... 0x0FFF:
                            tbl.name[1][addr & 0x03FF] = data;
                            break;
                    };
                }               
                break;
                
            // Palettes
            case 0x3F00 ... 0x3FFF:
                addr &= 0x001F;
                switch(addr)
                {
                    case 0x0010:
                        addr = 0x0000;
                        break;
                        
                    case 0x0014: 
                        addr = 0x0004;
                        break;

                    case 0x0018:
                        addr = 0x0008;
                        break;

                    case 0x001C:
                        addr = 0x000C;
                        break;
                };
                tbl.palettesMem[addr] = data;
                break;

        };
    };
}

uint8_t Ppu::cpuRead(uint16_t addr)
{
    // Read from CPU mapped registers
    uint8_t data = 0x00;
    switch (addr)
    {
        case 0x0000:
            break;

        case 0x0001:
            break;

        case 0x0002:
            data = (r.status & 0xE0) | (buffer & 0x1F);
            toggle = 0x00;
            setStatusBits(v, false);
            break;

        case 0x0003:
            break;
                
        case 0x0004:
            data = OAMdata[r.oam_addr];
            break;

        case 0x0005:
            break;

        case 0x0006:
            break;
            
        case 0x0007:
            data = buffer;
            buffer = ppuRead(currVRAM);

            if (currVRAM >= 0x3F00)
            {
                data = buffer;
            };

            if (getControllerFlags(I))
            {
                currVRAM += 0x0020;
            }
            else
            {
                currVRAM += 0x0001;
            };
            break;

        default:
            break;
        
    };
    return data;
}

void Ppu::cpuWrite(uint16_t addr, uint8_t data)
{
    // Write to CPU mapped registers
    switch (addr)
    {  

        case 0x0000:
            r.controller = data;
            setTVRAM(NAMETABLE_X, (uint16_t) getControllerFlags(n));
            setTVRAM(NAMETABLE_Y, (uint16_t) getControllerFlags(N));
            break;

        case 0x0001:
            r.mask = data;
            break;

        case 0x0002:
            break;

        case 0x0003:
            r.oam_addr = data;
            break;

        case 0x0004:
            break;

        case 0x0005:
            switch(toggle)
            {
                // X Scroll - 1st write
                case 0:
                    fine_X = data & 0x07;
                    setTVRAM(COARSE_X, (data >> 3));
                    toggle = 0x01;
                    break;

                // Y Scroll - 2nd write
                case 1:
                    setTVRAM(FINE_Y, (data & 0x07));
                    setTVRAM(COARSE_Y, (data >> 3));
                    toggle = 0x00;
                    break;

            };
            break;

        case 0x0006:
            switch(toggle)
            {
                case 0:
                    data &= 0x3F;
                    tempVRAM = (uint16_t) ((data) << 8) | (tempVRAM & 0x00FF);
                    toggle = 0x01;
                    break;

                case 1:
                    tempVRAM = (tempVRAM & 0xFF00) | (uint16_t) (data & 0x00FF);
                    currVRAM = tempVRAM;
                    toggle = 0x00;
                    break;
            };
            break;

        case 0x0007:
            ppuWrite(currVRAM, data);

            if(getControllerFlags(I))
            {
                currVRAM += 0x20;
            } 
            else
            {
                currVRAM += 0x01;
            }; 
            break;

        default:
            break; 
    };
}


// ---------------------------- MAPPED REGISTERS ---------------------------- //
// Controller flags manipulation
uint8_t Ppu::getControllerFlags(ControllerFlags cBits)
{
    if ((r.controller & cBits) > 0)
    {
        return 1;
    }
    else
    {
        return 0;
    };
}

void Ppu::setControllerFlags(ControllerFlags cBits, bool mode) 
{
    if(mode) 
    {
        r.controller = r.controller | cBits;    // Set Controller Flags
    }
    else 
    {
        r.controller = r.controller & ~cBits;   // Clear Controller Flags
    };
}

// Mask register manipulation
uint8_t Ppu::getMaskBits(MaskBits mBits)
{
    if ((r.mask & mBits) > 0)
    {
        return 1;
    }
    else 
    {
        return 0;
    };
}

void Ppu::setMaskBits(MaskBits mBits, bool mode)
{
    if(mode)
    {
        r.mask = r.mask | mBits;
    }
    else 
    {
        r.mask = r.mask & ~mBits;
    };
}

// Status register manipulations
uint8_t Ppu::getStatusBits(StatusBits sBits)
{
    if((r.status & sBits) > 0)
    {
        return 1;
    }
    else 
    {
        return 0;
    };
}

void Ppu::setStatusBits(StatusBits sBits, bool mode)
{
    if (mode)
    {
        r.status = r.status | sBits;
    }
    else
    {
        r.status = r.status & ~sBits;
    };
}

inline uint16_t Ppu::getVRAM(VRAMRegisters vreg)
{
    uint16_t reg = 0;
    switch (vreg)
    {
        case COARSE_X:
            reg = ((currVRAM & COARSE_X)) & 0x001F;
            break;
        
        case COARSE_Y:
            reg = ((currVRAM & COARSE_Y) >> 5) & 0x001F;
            break;

        case NAMETABLE_X:
            reg = ((currVRAM & NAMETABLE_X) >> 10) & 0x01;
            break;

        case NAMETABLE_Y:
            reg = ((currVRAM & NAMETABLE_Y) >> 11) & 0x01;
            break;

        case FINE_Y:
            reg = ((currVRAM & FINE_Y) >> 12) & 0x07;
            break;
        
        case VRAM_REG:
            reg = (currVRAM & VRAM_REG);
            break;
    };
    return reg;
}

inline void Ppu::setVRAM(VRAMRegisters vreg, uint16_t vreg_data)
{
    switch (vreg)
    {
        case COARSE_X:
            currVRAM &= 0x7FE0;
            currVRAM |= (vreg_data & COARSE_X);
            break;
        
        case COARSE_Y:
            currVRAM &= 0x7C1F;
            currVRAM |= ((vreg_data << 5) & COARSE_Y);
            break;

        case NAMETABLE_X:
            currVRAM &= 0x7BFF;
            currVRAM |= ((vreg_data << 10) & NAMETABLE_X);
            break;

        case NAMETABLE_Y:
            currVRAM &= 0x77FF;
            currVRAM |= ((vreg_data << 11) & NAMETABLE_Y);
            break;

        case FINE_Y:
            currVRAM &= 0x0FFF;
            currVRAM |= ((vreg_data << 12) & FINE_Y);
            break;
        
        case VRAM_REG:
            currVRAM = (vreg_data  & VRAM_REG);
            break;
    };
    currVRAM &= VRAM_REG;
}


inline uint16_t Ppu::getTVRAM(VRAMRegisters tvreg)
{
    uint16_t reg = 0;
    switch (tvreg)
    {
        case COARSE_X:
            reg = ((tempVRAM & COARSE_X)) & 0x001F;
            break;
        
        case COARSE_Y:
            reg = ((tempVRAM & COARSE_Y) >> 5) & 0x001F;
            break;

        case NAMETABLE_X:
            reg = ((tempVRAM & NAMETABLE_X) >> 10) & 0x01;
            break;

        case NAMETABLE_Y:
            reg = ((tempVRAM & NAMETABLE_Y) >> 11) & 0x01;
            break;

        case FINE_Y:
            reg = ((tempVRAM & FINE_Y) >> 12) & 0x07;
            break;
        
        case VRAM_REG:
            reg = (tempVRAM & VRAM_REG);
            break;
    };
    return reg;
}

inline void Ppu::setTVRAM(VRAMRegisters tvreg, uint16_t tvreg_data)
{
    switch (tvreg)
    {
        case COARSE_X:
            tempVRAM &= 0x7FE0;
            tempVRAM |= (tvreg_data & COARSE_X);
            break;
        
        case COARSE_Y:
            tempVRAM &= 0x7C1F;
            tempVRAM |= ((tvreg_data << 5) & COARSE_Y);
            break;

        case NAMETABLE_X:
            tempVRAM &= 0x7BFF;
            tempVRAM |= ((tvreg_data << 10) & NAMETABLE_X);
            break;

        case NAMETABLE_Y:
            tempVRAM &= 0x77FF;
            tempVRAM |= ((tvreg_data << 11) & NAMETABLE_Y);
            break;

        case FINE_Y:
            tempVRAM &= 0x0FFF;
            tempVRAM |= ((tvreg_data << 12) & FINE_Y);
            break;
        
        case VRAM_REG:
            tempVRAM = 0x00;
            tempVRAM = (tvreg_data & VRAM_REG);
            break;
    };
    tempVRAM &= VRAM_REG;
}


// ---------------------------- BACKGROUND ---------------------------- //
// Update Shift Registers - values from the latches are fed to the shift registers
void Ppu::loadIntoShiftRegisters()
{
    shift.patternLeft = (shift.patternLeft & 0xFF00) | fetched.patternLeft;
    shift.patternRight = (shift.patternRight & 0xFF00) | fetched.patternRight;

    if (fetched.tileAttr & 0x01)
    {
        shift.palAttribLow = (shift.palAttribLow & 0xFF00) | (0xFF);
    }
    else
    {
        shift.palAttribLow = (shift.palAttribLow & 0xFF00) | (0x00);
    };

    if (fetched.tileAttr & 0x02)
    {
        shift.palAttribHi = (shift.palAttribHi & 0xFF00) | (0xFF);
    }
    else
    {
        shift.palAttribHi = (shift.palAttribHi & 0xFF00) | (0x00);
    };

}

// Iterate through Shift Register data
inline void Ppu::iterateShiftRegisters()
{
    // If BKG rendering enabled
    if(getMaskBits(b))
    {
        // Move background tile pattern row in shift register by 1
        shift.patternLeft <<= 1;
        shift.patternRight <<= 1;

        // Move palette attributes in shift register by 1
        shift.palAttribLow <<= 1;
        shift.palAttribHi <<= 1;
    };
        
}

inline void Ppu::incrementCoarseX()
{
    if(getMaskBits(b) || getMaskBits(s))
    {
        if ((currVRAM & 0x001F) == 31)    // if coarse X == 31
        {
            currVRAM &= ~0x001F;          // coarse X = 0
            currVRAM ^= 0x0400;           // switch horizontal nametable
        }
        else
        {
            currVRAM += 1;                // increment coarse X
        };
    };
}

inline void Ppu::incrementY()
{
    if(getMaskBits(b) || getMaskBits(s))
    {
        // if fine Y < 7 - Increment fine y
        if ((currVRAM & 0x7000) != 0x7000)           
        {
            currVRAM += 0x1000;
        }
        else
        {
            currVRAM &= ~0x7000;

			// Check if we need to swap vertical nametable targets
            int y = (currVRAM & 0x03E0) >> 5;

            // Reset coarse y offset & Switch vertical nametable
			if (y == 29)
			{
				y = 0;
				currVRAM ^= 0x0800;
			}

            // coarse Y = 0, nametable not switched. wrap around the current nametable
			else if (y == 31)
			{
				y = 0;
			}
            // Else Increment the coarse y offset
			else
			{
				y += 1;
			};

            // Put y back into Current VRAM
            currVRAM = (currVRAM & ~0x03E0) | (y << 5);
        };
    };
}

// Reset X address - set to contents from temp vram
inline void Ppu::resetAddressX()
{
    if((getMaskBits(b)) || (getMaskBits(s)))
    {
        setVRAM(NAMETABLE_X, getTVRAM(NAMETABLE_X));
        setVRAM(COARSE_X, getTVRAM(COARSE_X));

    };
}

inline void Ppu::resetAddressY()
{
    // Reset Y address - set to contents from temp vram
    if((getMaskBits(b)) || (getMaskBits(s)))
    {
        setVRAM(FINE_Y, getTVRAM(FINE_Y));
        setVRAM(NAMETABLE_Y, getTVRAM(NAMETABLE_Y));
        setVRAM(COARSE_Y, getTVRAM(COARSE_Y));
    };

}

// Evaluate Background - bit is fetched from the 4 background shift registers in order to create a pixel on screen
void Ppu::fetchBackground()
{
    switch((cycles - 1) % 8)
    {
        // Update shift registers with latched data - takes 2 cycles to complete & Fetch next background tile nametable ID
        case 0:
            loadIntoShiftRegisters();
            fetched.tileNameTbl = ppuRead(0x2000 | (currVRAM & 0x0FFF));
            break;
                
        // Fetch next Background tile attribute                                
        case 2:
            fetched.tileAttr = ppuRead(0x23C0 | (currVRAM & 0x0C00) | ((currVRAM >> 4) & 0x38) | ((currVRAM >> 2) & 0x07));

            if (getVRAM(COARSE_Y) & 0x02)
            {
                fetched.tileAttr >>= 4;
            };

            if (getVRAM(COARSE_X) & 0x02)
            {
                fetched.tileAttr >>= 2;
            };

            fetched.tileAttr &= 0x03;
            break;
        
        // Fetch pattern background tile - Left Plane
        case 4:
            fetched.patternLeft = ppuRead((getControllerFlags(B) << 12) + ((uint16_t) fetched.tileNameTbl << 4) + (getVRAM(FINE_Y)));
            break;

        // Fetch patten background tile - Right Plane (Offset by 8 bits)
        case 6:
            fetched.patternRight = ppuRead((getControllerFlags(B) << 12) + ((uint16_t) fetched.tileNameTbl << 4) + (getVRAM(FINE_Y)) + 8);
            break;
                       
        // Switch to the next tile in Nametable memory 
        case 7:
            incrementCoarseX();
            break;
        
        default:
            break;
    };   
}

// Sprite Fetches - Cycles 257-320: (8 sprites total, 8 cycles per sprite)
void Ppu::fetchSprites()
{
    uint8_t thisCycle = cycles % 8;
    switch (thisCycle)
    {
        // Read the Y-Coordinate
        case 1:
            fetchedSprite.positionY = second_OAM[sOAM_counter].positionY;
            break;

        // Read the Tile Number
        case 2:
            fetchedSprite.tileIndex = second_OAM[sOAM_counter].tileIndex;
            break;

        // Read the Attributes
        case 3:
            fetchedSprite.attributes = second_OAM[sOAM_counter].attributes;
            break;

        // Read the X-Coordinate
        case 4:
            fetchedSprite.positionX = second_OAM[sOAM_counter].attributes;
            break;

        // Read the X-Coordinate of the selected sprite from secondary OAM 4 times (while the PPU fetches the sprite tile data)
        // Load Fetched Sprite data into Shift Registers - in one hit (NES doesn't do this)
        case 8:

            // Determine the type of Flip Performed on Fetched Sprite
            if(fetchedSprite.attributes & 0x40)
            {
                sprite_flip = HORIZONTAL;
            }
            else if (fetchedSprite.attributes & 0x80)
            {
                sprite_flip = VERTICAL;
            }
            else
            {
                sprite_flip = NORMAL;
            };
            

            // 8 x 16 Tiles - Flipping Logic
            if(getControllerFlags(H))
            {
                // Determine if reading hi or low of the 8x16 tile
                bool isloTile = (scanLine - fetchedSprite.positionY < 8);


                // Place Addr in Sprite Shift Registers
                if (sprite_flip == VERTICAL)
                {
                    sprShift[sOAM_counter].lo_patternAddr = ((fetchedSprite.tileIndex & 0b01) << 12)  // Bank ($0000 or $1000) of tiles
                        | (((fetchedSprite.tileIndex & 0xFE) + (uint8_t)isloTile) << 4)               // Tile column (16 byte offset)
						| (7 - (scanLine - fetchedSprite.positionY) & 0x07);                          // Fine Y offset

                }
                else if(sprite_flip == HORIZONTAL || sprite_flip == NORMAL)
                {
                    sprShift[sOAM_counter].lo_patternAddr = ((fetchedSprite.tileIndex & 0b01) << 12)   // Sprite table select - Half of sprite table (0: "left"; 1: "right"))
                        | (((fetchedSprite.tileIndex & 0xFE) + (uint8_t)isloTile) << 4)                // Tile column (16 byte offset)
						| ((scanLine - fetchedSprite.positionY) & 0x07);                               // Fine Y offset

                };

            }
            // 8 x 8 Tiles - Flipping Logic
            else
            {
                // Place Addr in Sprite Shift Registers
                if (sprite_flip == VERTICAL)
                {
                    sprShift[sOAM_counter].lo_patternAddr = (getControllerFlags(S) << 12)  // Sprite table select - Half of sprite table (0: "left"; 1: "right"))
                        | (fetchedSprite.tileIndex << 4)                                   // Tile column (16 byte offset)
						| (7 - (scanLine - fetchedSprite.positionY));                      // Fine Y offset
                }
                else if(sprite_flip == HORIZONTAL || sprite_flip == NORMAL)
                {
                    sprShift[sOAM_counter].lo_patternAddr = (getControllerFlags(S) << 12)  // Sprite table select - Half of sprite table (0: "left"; 1: "right"))
                        | (fetchedSprite.tileIndex << 4)                                   // Tile column (16 byte offset)
						| (scanLine - fetchedSprite.positionY);                            // Fine Y offset  

                };

            };
            // Read from Sprite memory addr
            sprShift[sOAM_counter].lo_patternBit = ppuRead(sprShift[sOAM_counter].lo_patternAddr);
            sprShift[sOAM_counter].hi_patternBit = ppuRead((sprShift[sOAM_counter].lo_patternAddr) + 0x0008);

            // Apply the Horizontal Flip to the Pattern Plane Once bits are read from PPU Bus
            if (sprite_flip == HORIZONTAL)
            {
                std::bitset<16U> flipped_pattern = 0x0000;

                // Apply to Low pattern
                flipped_pattern = sprShift[sOAM_counter].lo_patternAddr; 
                flipped_pattern = flipped_pattern.flip();
                sprShift[sOAM_counter].lo_patternBit = (uint16_t) flipped_pattern.to_ulong();

                // Apply to High pattern
                flipped_pattern = 0x0000;
                flipped_pattern = sprShift[sOAM_counter].hi_patternAddr; 
                flipped_pattern = flipped_pattern.flip();
                sprShift[sOAM_counter].hi_patternBit = (uint16_t) flipped_pattern.to_ulong();
            };

            // Inmcrement sOAM counter each 8th cycle
            sOAM_counter++;
            break;

        default:
            break;
    };

}


// Checks range & size of Sprite
inline bool Ppu::spriteRangeCheck()
{
    // Ensure data boundary Check
    if (sprites_found < 8 && sOAM_counter < 8 && pOAM_counter < 64)
    {
        int sprSize = 8;

        // Check Y-Coordinate against the scanline location
        int range = ((int)scanLine - (int) OAM[pOAM_counter].positionY) >= 0;

        // Check for Double Sprite size
        if(getControllerFlags(H))
        {
            sprSize = 16;
        };

        // Range check against the sprite boundary
        if (range < sprSize)
        {
            return true;
        }
    };

    return false;
}


// ---------------------------- RENDERING ---------------------------- //
void Ppu::setScreenPixels()
{
    uint8_t bkg_pattern = 0x00;
    uint8_t bkg_palette = 0x00;

    if(getMaskBits(b) && (getMaskBits(m) || cycles >= 9))
    {
        // Offset with Fine X
        uint16_t xOffset = 0x8000 >> fine_X;

        // Get patterns with offset value
        uint8_t loPattern = (shift.patternLeft & xOffset) > 0;
        uint8_t hiPattern = (shift.patternRight & xOffset) > 0;
        bkg_pattern = (hiPattern << 1) | loPattern;

        // Get palette attributes
        uint8_t loPal = (shift.palAttribLow & xOffset) > 0;
        uint8_t hiPal = (shift.palAttribHi & xOffset) > 0;
        bkg_palette = (hiPal << 1) | loPal;
    };

    if (getMaskBits(s) && (getMaskBits(M) || cycles >= 9))
    {
        int spr = (int) sprites_found;
        while(spr >= 0)
        {
            spr--;
        };
    }

    palAddr = ppuRead(0x3F00 + (bkg_palette << 2) + bkg_pattern) & 0x3F;

    // Get RGB values from color palette
    renderPixel.red = tbl.colors[palAddr].red;
    renderPixel.green = tbl.colors[palAddr].green;
    renderPixel.blue = tbl.colors[palAddr].blue;
    renderPixel.X = cycles - 1;
    renderPixel.Y = scanLine;

    // Render Pixel to Screen through Game Engine
    if (scanLine >= 0 && scanLine <= 239)
    {
        if(cycles >= 1 && cycles <= 256)
        {
            screenPixels[pixel_counter] = renderPixel;
            pixel_counter++;

            if (pixel_counter == 61440)
            {
                pixel_counter = 0;
            };
        };
    };
}

// Render Frame - 256 scanlines
void Ppu::render()
{
    // Background & Sprite Rendering
    switch (scanLine)
    {
        // Pre-render Scanline & Visible Scanlines
        case -1 ... 239:

            // New Frame
            if (scanLine == -1 || scanLine == 0)
            {
                frameComplete = false;

                // Clear Overflow Flag - Overflow bug
                if (scanLine == -1 && cycles == 1 && sprites_found < 8)
                {
                    setStatusBits(O, false);
                    sprites_found = 0;
                };
            };

            // Odd Frame
            if (scanLine == 0 && cycles == 0)
            {
                cycles = 1;
            };

            // Clear vertical blank
            if (scanLine == -1 && cycles == 1)
            {
                setStatusBits(v, false);
            };


            // Background Tile Fetching & Sprite Tile Fetching
            switch(cycles)
            {
                // Place fetched data into internal latches - Every Cycle
                case 1 ... 257:
                    if (cycles > 1)
                    {
                        iterateShiftRegisters();    // Place fetched data into internal latches - Every Cycle
                    };

                    fetchBackground();              // Background tile fetching

                    if (cycles == 256)
                    {
                        incrementY();               // Increment coarse Y
                    }
                    else if (cycles == 257)
                    {
                        loadIntoShiftRegisters();
                        resetAddressX();
                        //fetchSprites();
                    };
                    break;
                        
                // Cycles 257 - 320 : The tile data for the sprites on the next scanline are fetched 
                case 258 ... 320:
                    //fetchSprites();
                    break;

                // Cycles 321 - 336 : The first two tiles for next scanline are fetched & loaded into the shift registers
                // Background Render Pipeline initialization
                case 321 ... 337:
                    if (cycles > 1)
                    {
                        iterateShiftRegisters(); 
                    };    
                    fetchBackground();
                    break;

                // Cycles 338 - 340 : Two bytes are fetched - purpose for this is unknown
                case 338 ... 340:
                    if(cycles == 338 || cycles == 340)
                    {
                        fetched.tileNameTbl = ppuRead(0x2000 | (currVRAM & 0x0FFF));
                    };
                    break;

                default:
                    break;
            };

            // End of Vertical Banking period - Reset the Y address ready for rendering
            if (scanLine == -1 && cycles >= 280 && cycles < 305)
            {
                resetAddressY();
            };


            //if(scanLine < 0)
            //{
            //    break;
            //}

            // Sprite Evaluation - During Visible Scanlines
            switch (cycles)
            {
                // Initialize Secondary OAM - (every 4 cycles to account for vector. Original NES does not do this)
                case 1 ... 64: 
                    if (((cycles + 1) % 4) == 0)
                    {
                        for (int i = 0; i < 8; i++)
                        {
                            second_OAM[i].positionY = 0xFF;
                            second_OAM[i].tileIndex = 0xFF;
                            second_OAM[i].attributes = 0xFF;
                            second_OAM[i].positionX = 0xFF;
                        };
                    };
                    break;

                // Sprite Evaluation - Evaluate the sprites on next scanline (more than 8 is Sprite overflow)
                case 65 ... 256: 

                    // Primary OAM Data is written to secondary OAM on Even Cycles
                    if (cycles % 2 == 0)
                    {
                        // If Y-coordinate is in Range & Not Full/Overflowed -> Write to second_OAM
                        if (spriteRangeCheck())
                        {
                            second_OAM[sOAM_counter].positionY = OAM[pOAM_counter].positionY;
                            second_OAM[sOAM_counter].tileIndex = OAM[pOAM_counter].tileIndex;
                            second_OAM[sOAM_counter].attributes = OAM[pOAM_counter].attributes;
                            second_OAM[sOAM_counter].positionX = OAM[pOAM_counter].positionX;

                            // Increment sOAM counter & sprite found
                            sOAM_counter++;
                            sprites_found++; 

                            // Set Sprite Overflow Flag (cleared at dot 1 (the second dot) of the pre-render line)
                            //setStatusBits(O, true);

                        } 
                        // Sprite Overflow
                        else if (sprites_found > 8)
                        {
                            // Sprite Overflow Flag still set during overflow
                            setStatusBits(O, true);
                        }
                    }
                    // Odd Cycles - Data is read from (primary) OAM
                    else {break;};

                    // Increment primary OAM counter (n)
                    pOAM_counter++;

                    // Reset OAM Counters
                    if(cycles == 256)
                    {
                        sOAM_counter = 0;
                        pOAM_counter = 0;
                    }
                    break;
                
                default:
                    break;
            };
            break;



        // PPU Idle Scanline - PPU just idles during this scanline
        case 240:
            break;


        // Vertical Blanking Scanlines - VBlank flag of the PPU is set at tick 1 (the second tick) of scanline 241, where the VBlank NMI also occurs
        case 241 ... 260:   
            if (cycles == 1 && scanLine == 241)
            {
                setStatusBits(v, true);
                if (getControllerFlags(V))
                {
                    NMI = true;
                };
            };
            break;   

        default:
            break;
    };



    // Get Pixel Values for rednering - Render Pixel to Screen through Game Engine
    if (scanLine >= 0 && scanLine <= 239)
    {
        if(cycles >= 1 && cycles <= 256)
        {
            setScreenPixels();
        };
    };
    



    // Iterate Frame
    cycles++;
    if(cycles >= 341)
    {
        cycles = 0;
        scanLine++;

        if (scanLine >= 261)
        {
            frameComplete = true;
            scanLine = -1;
        };
    };

}