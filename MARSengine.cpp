#include "MARSengine.h"

MARS::MARS() 
{
    init();
}

MARS::~MARS(){}

void MARS::keyboardInput(SDL_Event &event)
{
    //SDL_Delay(500);
    switch(event.key.keysym.sym)
    {
        case SDLK_SPACE:
            if(debMode == true)
            {
                nes.systemClock();
                //logWrite();

                drawScreen();

            };
            break;
        
        case SDLK_RIGHT:
            if (selectedPalette < 0x07)
            {
                selectedPalette++;
            }
            else
            {
                selectedPalette = 0;
            }
            resetPatternTables(selectedPalette);
            break;
        
        case SDLK_LEFT:
            if (selectedPalette == 0)
            {
                selectedPalette = 0x07;
            }
            else
            {
                selectedPalette--;
            }
            resetPatternTables(selectedPalette);
            break;

        case SDLK_DOWN:
            break;

        case SDLK_UP:
            break;

        case SDLK_p:
            drawPatternTables();
            break;

        case SDLK_r:
            nes.systemReset();
            SDL_RenderClear(rend_debug);
            drawScreen();

        // Render a Whole Frame
        case SDLK_RETURN:
            if (debMode == true)
            {
                while(nes.ppu.frameComplete == false)
                {
                    nes.systemClock();
                    logWrite();
                }
                drawScreen();
            }
            break;

        // Display RAM Contents
        case SDLK_PAGEDOWN:
            SDL_RenderClear(rend_debug);
            drawRam();
            break;
        
        // Quit Application
        case SDLK_ESCAPE:
            SDL_DestroyRenderer(renderer);
            SDL_Quit();
            break;

        default:
            break;

    }
}

void MARS::getControllerState()
{
    const uint8_t *controller = SDL_GetKeyboardState(NULL);
    uint8_t bits = 0x00;

    if (controller[SDL_SCANCODE_D])
    {
        bits |= RIGHT;
    };

    if (controller[SDL_SCANCODE_A])
    {
        bits |= LEFT;
    };

    if (controller[SDL_SCANCODE_S])
    {
        bits |= DOWN;
    };

    if (controller[SDL_SCANCODE_W])
    {
        bits |= UP;
    };

    if (controller[SDL_SCANCODE_E])
    {
        bits |= START;
    };

    if (controller[SDL_SCANCODE_Q])
    {
        bits |= SELECT;
    };

    if (controller[SDL_SCANCODE_K])
    {
        bits |= B;
    };

    if (controller[SDL_SCANCODE_M])
    {
        bits |= A;
    };
    bits &= 0xFF;
    nes.setController(bits, 0);
}


inline void MARS::drawCpu()
{
    // PC Text
    const char *pcText = "PC:";
    surface = TTF_RenderText_Solid(lg_font, pcText, Blue);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    box.x = RIGHT_L_JUSTIFIED;
    box.y = 650;
    box.w = 100;
    box.h = 35;
    SDL_RenderCopy(renderer, texture, NULL, &box);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);

    // PC Value
    text_ss.str( std::string() );
    text_ss.clear();
    text_ss << std::hex << nes.cpu.reg.PC;
    const char *pc = text_ss.str().c_str();
    surface = TTF_RenderText_Solid(sm_font, pc, White);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    box.x = RIGHT_R_JUSTIFIED;
    box.y = 650;
    box.w = 150;
    box.h = 35;
    SDL_RenderCopy(renderer, texture, NULL, &box);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);

    // Accumulator Text
    const char *aText = "A:";
    surface = TTF_RenderText_Solid(lg_font, aText, Blue);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    box.x = RIGHT_L_JUSTIFIED;
    box.y = 700;
    box.w = 50;
    box.h = 35;
    SDL_RenderCopy(renderer, texture, NULL, &box);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);

    // Accumulator Value
    text_ss.str( std::string() );
    text_ss.clear();
    text_ss << std::hex << (int)nes.cpu.reg.A;
    const char *a = text_ss.str().c_str();
    surface = TTF_RenderText_Solid(sm_font, a, White);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    box.x = RIGHT_R_JUSTIFIED;
    box.y = 700;
    box.w = 75;
    box.h = 35;
    SDL_RenderCopy(renderer, texture, NULL, &box);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);

    // X Register Text
    //std::string s_x = "X:";
    const char *xText = "X:";
    surface = TTF_RenderText_Solid(lg_font, xText, Blue);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    box.x = RIGHT_L_JUSTIFIED;
    box.y = 750;
    box.w = 50;
    box.h = 35;
    SDL_RenderCopy(renderer, texture, NULL, &box);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);

    // X Register Value
    text_ss.str( std::string() );
    text_ss.clear();
    text_ss << std::hex << (int)nes.cpu.reg.X;
    const char *x = text_ss.str().c_str();
    surface = TTF_RenderText_Solid(sm_font, x, White);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    box.x = RIGHT_R_JUSTIFIED;
    box.y = 750;
    box.w = 75;
    box.h = 35;
    SDL_RenderCopy(renderer, texture, NULL, &box);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);



    // Y Register Text
    const char *yText = "Y:";
    surface = TTF_RenderText_Solid(lg_font, yText, Blue);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    box.x = RIGHT_L_JUSTIFIED;
    box.y = 800;
    box.w = 50;
    box.h = 35;
    SDL_RenderCopy(renderer, texture, NULL, &box);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);

    // Y Register Value
    text_ss.str( std::string() );
    text_ss.clear();
    text_ss << std::hex << (int)nes.cpu.reg.Y;
    const char *y = text_ss.str().c_str();
    surface = TTF_RenderText_Solid(sm_font, y, White);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    box.x = RIGHT_R_JUSTIFIED;
    box.y = 800;
    box.w = 75;
    box.h = 35;
    SDL_RenderCopy(renderer, texture, NULL, &box);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);


    // Stack Text
    const char *stackText = "STACK:";
    surface = TTF_RenderText_Solid(lg_font, stackText, Blue);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    box.x = RIGHT_L_JUSTIFIED;
    box.y = 850;
    box.w = 125;
    box.h = 35;
    SDL_RenderCopy(renderer, texture, NULL, &box);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);

    // Stack Value
    text_ss.str( std::string() );
    text_ss.clear();
    text_ss << std::hex << (int)nes.cpu.reg.S;
    const char *stack = text_ss.str().c_str();
    surface = TTF_RenderText_Solid(sm_font, stack, White);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    box.x = RIGHT_R_JUSTIFIED;
    box.y = 850;
    box.w = 75;
    box.h = 35;
    SDL_RenderCopy(renderer, texture, NULL, &box);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);



    // Status Flag Text
    const char *statusText = "STATUS:";
    surface = TTF_RenderText_Solid(lg_font, statusText, Blue);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    box.x = RIGHT_L_JUSTIFIED;
    box.y = 900;
    box.w = 125;
    box.h = 35;
    SDL_RenderCopy(renderer, texture, NULL, &box);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);

    // Status Flag Values
    std::bitset<8> status = nes.cpu.reg.P;
    X = RIGHT_R_JUSTIFIED;
    Y = 900;
    W = 25;
    H = 35;
    
    for(int i = 0; i <= 8; i++)
    {
        std::string s_p = status.to_string();
        char bit = s_p[i];
        s_p = bit;
        const char *statusVal = s_p.c_str();
        
        if(s_p== "0")
        {
            surface = TTF_RenderText_Solid(sm_font, statusVal, Red);
        }
        else
        {
            surface = TTF_RenderText_Solid(sm_font, statusVal, Green);
        }
        texture = SDL_CreateTextureFromSurface(renderer, surface);
        box.x = X;
        box.y = Y;
        box.w = W;
        box.h = H;
        X += 25;
        SDL_RenderCopy(renderer, texture, NULL, &box);
        SDL_FreeSurface(surface);
        SDL_DestroyTexture(texture);
    };

    text_ss.str( std::string() );
    text_ss.clear();
    text_ss << std::hex << (int)nes.cpu.reg.P;
    const char *stat= text_ss.str().c_str();

    surface = TTF_RenderText_Solid(sm_font, stat, White);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    box.x = X + 10;
    box.y = Y;
    box.w = W;
    box.h = H;
    SDL_RenderCopy(renderer, texture, NULL, &box);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);

    // Render CPU Elements
    //SDL_RenderPresent(renderer);
}

inline void MARS::drawRam()
{
    X = 5;
    Y = 128;
    W = 25;
    H = 20;

    // Get RAM contents from NES
    int ram_size = 2048;
    std::string ram;

    // Place RAM into String
    for (int i = 0; i < ram_size; i++)
    {
        ram = std::to_string(nes.cpuRam[i]);
        const char *ramVal = ram.c_str();

        // Render RAM Contents
        surface = TTF_RenderText_Solid(sm_font, ramVal, White);
        texture = SDL_CreateTextureFromSurface(rend_debug, surface);
        box.x = X;
        box.y = Y;
        box.w = W;
        box.h = H;
        
        X += 28;

        // Reset Y coordinate 
        if(i % 64 == 0)
        {
            X = 5;
            Y += 20;
        }
        SDL_RenderCopy(rend_debug, texture, NULL, &box);

        SDL_FreeSurface(surface);
        SDL_DestroyTexture(texture);
        ram.clear();
    };
    SDL_RenderPresent(rend_debug);

}


inline void MARS::drawPpu()
{
    const char *cText = "Controller: ";
    const char *mText = "Mask: ";
    const char *sText = "Status: ";

    std::bitset<8> mask = nes.ppu.r.mask;
    std::bitset<8> status = nes.ppu.r.status;


    // Controller Register Text & Values
    surface = TTF_RenderText_Solid(lg_font, cText, Blue);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    box.x = RIGHT_L_JUSTIFIED;
    box.y = 300;
    box.w = 175;
    box.h = 35;

    SDL_RenderCopy(renderer, texture, NULL, &box);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);

    X = RIGHT_R_JUSTIFIED;
    Y = 300;
    W = 25;
    H = 35;
    
    std::bitset<8> controller = nes.ppu.r.controller;
    //std::bitset<8> controller = 0x0F;
    for(int i = 0; i <= 8; i++)
    {
        std::string s_controller = controller.to_string();
        char bit = s_controller[i];
        s_controller = bit;
        const char *controllerVal = s_controller.c_str();
        
        if(s_controller == "0")
        {
            surface = TTF_RenderText_Solid(sm_font, controllerVal, Red);
        }
        else
        {
            surface = TTF_RenderText_Solid(sm_font, controllerVal, Green);
        }
        texture = SDL_CreateTextureFromSurface(renderer, surface);
        box.x = X;
        box.y = Y;
        box.w = W;
        box.h = H;
        X += 25;

        SDL_RenderCopy(renderer, texture, NULL, &box);
        SDL_FreeSurface(surface);
        SDL_DestroyTexture(texture);
    };



    // Mask Register Text & Values
    surface = TTF_RenderText_Solid(lg_font, mText, Blue);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    box.x = RIGHT_L_JUSTIFIED;
    box.y = 350;
    box.w = 175;
    box.h = 35;

    SDL_RenderCopy(renderer, texture, NULL, &box);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);

    X = RIGHT_R_JUSTIFIED;
    Y = 350;
    W = 25;
    H = 35;

    for(int i = 0; i <= 8; i++)
    {
        std::string s_mask = mask.to_string();
        char bit = s_mask[i];
        s_mask = bit;
        const char *maskVal = s_mask.c_str();
        
        if(s_mask == "0")
        {
            surface = TTF_RenderText_Solid(sm_font, maskVal, Red);
        }
        else
        {
            surface = TTF_RenderText_Solid(sm_font, maskVal, Green);
        }
        texture = SDL_CreateTextureFromSurface(renderer, surface);
        box.x = X;
        box.y = Y;
        box.w = W;
        box.h = H;
        X += 25;

        SDL_RenderCopy(renderer, texture, NULL, &box);
        SDL_FreeSurface(surface);
        SDL_DestroyTexture(texture);
    };

    // Status Register
    surface = TTF_RenderText_Solid(lg_font, sText, Blue);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    box.x = RIGHT_L_JUSTIFIED;
    box.y = 400;
    box.w = 175;
    box.h = 35;

    SDL_RenderCopy(renderer, texture, NULL, &box);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
    
    X = RIGHT_R_JUSTIFIED;
    Y = 400;
    W = 25;
    H = 35;

    for (int i = 0; i <= 8; i++)
    {
        std::string s_status = status.to_string();
        char bit = s_status[i];
        s_status = bit;
        const char *statusVal = s_status.c_str();

        if (s_status == "0")
        {
            surface = TTF_RenderText_Solid(sm_font, statusVal, Red);
        }
        else
        {
            surface = TTF_RenderText_Solid(sm_font, statusVal, Green);
        }

        texture = SDL_CreateTextureFromSurface(renderer, surface);
        box.x = X;
        box.y = Y;
        box.w = W;
        box.h = H;
        X += 25;

        SDL_RenderCopy(renderer, texture, NULL, &box);
        SDL_FreeSurface(surface);
        SDL_DestroyTexture(texture);
    }
    /*

    // PPU Scanline Text
    const char *scanlineText = "Scanline: ";
    surface = TTF_RenderText_Solid(lg_font, scanlineText, Blue);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    box.x = RIGHT_L_JUSTIFIED;
    box.y = 450;
    box.w = 150;
    box.h = 35;
    SDL_RenderCopy(renderer, texture, NULL, &box);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);

    // PPU Scanline Value
    std::string s_scanline = to_string(nes.ppu.scanLine);
    const char *scanline = s_scanline.c_str();
    surface = TTF_RenderText_Solid(lg_font, scanline, Blue);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    box.x = RIGHT_R_JUSTIFIED;
    box.y = 450;
    box.w = 100;
    box.h = 35;
    SDL_RenderCopy(renderer, texture, NULL, &box);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);


    // PPU Cycle Text
    const char *cycleText = "Cycle: ";
    surface = TTF_RenderText_Solid(lg_font, cycleText, Blue);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    box.x = RIGHT_L_JUSTIFIED;
    box.y = 500;
    box.w = 150;
    box.h = 35;
    SDL_RenderCopy(renderer, texture, NULL, &box);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);

    // PPU Cycle Value
    std::string s_cycle = to_string(nes.ppu.cycles);
    const char *cycle = s_cycle.c_str();
    surface = TTF_RenderText_Solid(lg_font, cycle, Blue);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    box.x = RIGHT_R_JUSTIFIED;
    box.y = 500;
    box.w = 100;
    box.h = 35;
    SDL_RenderCopy(renderer, texture, NULL, &box);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
    */

}


inline void MARS::drawSystemComponents()
{
    int global_count = nes.NES_SystemClock;

    // Draw MARS Title
    //std::string s_marsTitle = "M.A.R.S";
    const char *marsTitle = "M.A.R.S";
    surface = TTF_RenderText_Solid(lg_font, marsTitle, Red);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    box.x = 150;
    box.y = 10;
    box.w = 250;
    box.h = 75;
    SDL_RenderCopy(renderer, texture, NULL, &box);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);

    //std::string s_marsSubTitle = "Marcus's Arcade System";
    const char *marsSubTitle = "Marcus's Arcade System";
    surface = TTF_RenderText_Solid(sm_font, marsSubTitle, Red);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    box.x = 150;
    box.y = 100;
    box.w = 250;
    box.h = 45;
    SDL_RenderCopy(renderer, texture, NULL, &box);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);

    // Draw Selected Console
    const char *consoleText = "Console: ";
    surface = TTF_RenderText_Solid(sm_font, consoleText, Blue);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    box.x = LEFT_L_JUSTIFIED;
    box.y = 200;
    box.w = 200;
    box.h = 45;
    SDL_RenderCopy(renderer, texture, NULL, &box);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);

    const char *console = selectedConsole.c_str();
    surface = TTF_RenderText_Solid(sm_font, console, White);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    box.x = LEFT_R_JUSTIFIED;
    box.y = 200;
    box.w = 150;
    box.h = 45;
    SDL_RenderCopy(renderer, texture, NULL, &box);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);


    // Draw Selected Game
    const char *gameText = "Game: ";
    surface = TTF_RenderText_Solid(sm_font, gameText, Blue);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    box.x = LEFT_L_JUSTIFIED;
    box.y = 250;
    box.w = 200;
    box.h = 45;
    SDL_RenderCopy(renderer, texture, NULL, &box);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);

    const char *game = selectedGame.c_str();
    surface = TTF_RenderText_Solid(sm_font, game, White);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    box.x = LEFT_R_JUSTIFIED;
    box.y = 250;
    box.w = 200;
    box.h = 45;
    SDL_RenderCopy(renderer, texture, NULL, &box);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);


    // Draw Global Counter Text
    //std::string str = "Global Cycle:";
    const char *gCountText = "Global Cycle:";
    surface = TTF_RenderText_Solid(lg_font, gCountText, Blue);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    box.x = RIGHT_L_JUSTIFIED;
    box.y = 1000;
    box.w = 125;
    box.h = 35;
    SDL_RenderCopy(renderer, texture, NULL, &box);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);

    // Draw Global Counter Value
    std::string str2 = to_string(global_count);
    const char *gCountVal = str2.c_str();
    surface = TTF_RenderText_Solid(lg_font, gCountVal, White);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    box.x = RIGHT_R_JUSTIFIED;
    box.y = 1000;
    box.w = 150;
    box.h = 35;
    SDL_RenderCopy(renderer, texture, NULL, &box);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);


    // Draw Instruction Text
    //std::string str3 = "Instruction:";
    const char *instructText = "Instruction:";
    surface = TTF_RenderText_Solid(lg_font, instructText, Blue);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    box.x = RIGHT_L_JUSTIFIED;
    box.y = 1050;
    box.w = 150;
    box.h = 35;
    SDL_RenderCopy(renderer, texture, NULL, &box);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);

    // Draw Instruction Value
    std::string instruction = nes.cpu.d.get_instruction;
    const char *instructVal = instruction.c_str();
    surface = TTF_RenderText_Solid(lg_font, instructVal, White);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    box.x = RIGHT_R_JUSTIFIED;
    box.y = 1050;
    box.w = 100;
    box.h = 35;
    SDL_RenderCopy(renderer, texture, NULL, &box);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);


    // Draw Address Mode Text
    //std::string str4 = "Addr Mode:";
    const char *addrmodeText = "Addr Mode:";
    surface = TTF_RenderText_Solid(lg_font, addrmodeText, Blue);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    box.x = RIGHT_L_JUSTIFIED;
    box.y = 1100;
    box.w = 150;
    box.h = 35;
    SDL_RenderCopy(renderer, texture, NULL, &box);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);

    // Draw Address Mode Value
    std::string addrm = nes.cpu.d.get_addrmode;
    const char *addrmode = addrm.c_str();
    surface = TTF_RenderText_Solid(lg_font, addrmode, White);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    box.x = RIGHT_R_JUSTIFIED;
    box.y = 1100;
    box.w = 100;
    box.h = 35;
    SDL_RenderCopy(renderer, texture, NULL, &box);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);


    // Draw Opcodes Text
    //std::string str5 = "Opcodes:";
    const char *optext = "Opcodes:";
    surface = TTF_RenderText_Solid(lg_font, optext, Blue);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    box.x = RIGHT_L_JUSTIFIED;
    box.y = 1150;
    box.w = 125;
    box.h = 35;
    SDL_RenderCopy(renderer, texture, NULL, &box);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);

    // Draw Opcodes Value
    text_ss.str( std::string() );
    text_ss.clear();
    text_ss << std::hex << (int)nes.cpu.d.get_opcode;
    text_ss << " " << nes.cpu.d.get_opcode_instruction[0];
    text_ss << " " << nes.cpu.d.get_opcode_instruction[1];
    const char *opcode = text_ss.str().c_str();
    
    surface = TTF_RenderText_Solid(lg_font, opcode, White);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    box.x = RIGHT_R_JUSTIFIED;
    box.y = 1150;
    box.w = 200;
    box.h = 35;
    SDL_RenderCopy(renderer, texture, NULL, &box);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);


    // Draw Disassembly Log Title
    //const char *logTitle = "- Disassembly -";
    surface = TTF_RenderText_Solid(lg_font, "~ Disassembly ~", Red);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    box.x = 150;
    box.y = 350;
    box.w = 250;
    box.h = 25;
    SDL_RenderCopy(renderer, texture, NULL, &box);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);

    // Draw System Disassembly
    auto vDisassembly = nes.cpu.d.disassembly;
    int j = 0;
    X = 75;
    Y = 400;
    W = 425;
    H = 30;

    for(auto i = 0; i != vDisassembly.size(); i++)
    {
        const char* disassembly = vDisassembly[i].c_str();

        surface = TTF_RenderText_Solid(sm_font, disassembly, White);
        texture = SDL_CreateTextureFromSurface(renderer, surface);
        box.x = X;
        box.y = Y;
        box.w = W;
        box.h = H;
        SDL_RenderCopy(renderer, texture, NULL, &box);
        SDL_FreeSurface(surface);
        SDL_DestroyTexture(texture);
        Y += 35;
        j++;

    }; 

    // Draw NES Bus Title
    /*
    const char *busTitle = "- Bus Contents-";
    surface = TTF_RenderText_Solid(lg_font, busTitle, Red);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    box.x = 150;
    box.y = 1000;
    box.w = 200;
    box.h = 25;
    SDL_RenderCopy(renderer, texture, NULL, &box);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
    */

    /*
    // Draw Addr Text on Bus
    const char *addrText = "Addr:";
    surface = TTF_RenderText_Solid(lg_font, addrText, Blue);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    box.x = 100;
    box.y = 1050;
    box.w = 100;
    box.h = 25;
    SDL_RenderCopy(renderer, texture, NULL, &box);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);

    // Draw Addr Value on Bus
    text_ss.str( std::string() );
    text_ss.clear();
    text_ss << std::hex << (int)nes.bus_addr;
    const char *busaddr = text_ss.str().c_str();
    surface = TTF_RenderText_Solid(lg_font, busaddr, White);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    box.x = 225;
    box.y = 1050;
    box.w = 150;
    box.h = 25;
    SDL_RenderCopy(renderer, texture, NULL, &box);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);

    // Draw Data Text on Bus
    const char *dataText = "Data:";
    surface = TTF_RenderText_Solid(lg_font, dataText, Blue);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    box.x = 100;
    box.y = 1080;
    box.w = 100;
    box.h = 25;
    SDL_RenderCopy(renderer, texture, NULL, &box);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);

    // Draw Data Value on Bus
    text_ss.str( std::string() );
    text_ss.clear();
    text_ss << std::hex << (int)nes.bus_data;
    const char *busdata = text_ss.str().c_str();
    surface = TTF_RenderText_Solid(lg_font, busdata, White);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    box.x = 225;
    box.y = 1080;
    box.w = 75;
    box.h = 25;
    SDL_RenderCopy(renderer, texture, NULL, &box);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);

    // Draw Access Type Text on Bus
    const char *aTypeText = "Access Type:";
    surface = TTF_RenderText_Solid(lg_font, aTypeText, Blue);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    box.x = 100;
    box.y = 1110;
    box.w = 100;
    box.h = 25;
    SDL_RenderCopy(renderer, texture, NULL, &box);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);

    // Draw Access Type Value on Bus
    const char *aTypeVal = nes.bus_accessType.c_str();
    surface = TTF_RenderText_Solid(lg_font, aTypeVal, Green);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    box.x = 225;
    box.y = 1110;
    box.w = 200;
    box.h = 25;
    SDL_RenderCopy(renderer, texture, NULL, &box);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
    */

    // Draw Controller Text
    const char *controlText = "Controller";
    surface = TTF_RenderText_Solid(lg_font, controlText, Blue);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    box.x = 100;
    box.y = 1160;
    box.w = 100;
    box.h = 25;
    SDL_RenderCopy(renderer, texture, NULL, &box);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);

    // Draw Controller Value
    std::bitset<8> control = (int)nes.controllers[0];
    std::string button = "RLDUsSBA";
    X = 225;
    Y = 1160;
    W = 25;
    H = 25;

    for(int i = 0; i <= 8; i++)
    {
        std::string s_controlAlp;
        std::string s_controlVal = control.to_string();

        char controlBit = button[i];
        s_controlAlp = controlBit;
        const char *controlVal = s_controlAlp.c_str();

        if(control[i] == 0)
        {
            surface = TTF_RenderText_Solid(sm_font, controlVal, Red);
        }
        else
        {
            surface = TTF_RenderText_Solid(sm_font, controlVal, Green);
        };
        texture = SDL_CreateTextureFromSurface(renderer, surface);
        box.x = X;
        box.y = Y;
        box.w = W;
        box.h = H;
        X += 25;
        SDL_RenderCopy(renderer, texture, NULL, &box);
        SDL_FreeSurface(surface);
        SDL_DestroyTexture(texture);
    };


    // Draw Controller State Text
    const char *controlStateText = "Controller State";
    surface = TTF_RenderText_Solid(lg_font, controlStateText, Blue);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    box.x = 100;
    box.y = 1200;
    box.w = 100;
    box.h = 25;
    SDL_RenderCopy(renderer, texture, NULL, &box);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);

    // Draw Controller State Value
    std::bitset<8> controlState = (int)nes.controllerState[0];
    X = 225;
    Y = 1200;
    W = 25;
    H = 25;

    for(int i = 0; i <= 8; i++)
    {
        std::string s_controlStateVal = controlState.to_string();

        char controlStateBit = s_controlStateVal[i];
        s_controlStateVal = controlStateBit;
        const char *controlStateVal = s_controlStateVal.c_str();

        if(control[i] == 0)
        {
            surface = TTF_RenderText_Solid(sm_font, controlStateVal, Red);
        }
        else
        {
            surface = TTF_RenderText_Solid(sm_font, controlStateVal, Green);
        };
        texture = SDL_CreateTextureFromSurface(renderer, surface);
        box.x = X;
        box.y = Y;
        box.w = W;
        box.h = H;
        X += 25;
        SDL_RenderCopy(renderer, texture, NULL, &box);
        SDL_FreeSurface(surface);
        SDL_DestroyTexture(texture);
    };


    // Draw FPS Text
    const char *fpsText = "FPS";
    surface = TTF_RenderText_Solid(sm_font, fpsText, Red);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    box.x = 2325;
    box.y = 50;
    box.w = 70;
    box.h = 50;
    SDL_RenderCopy(renderer, texture, NULL, &box);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
    

    // Draw Framerate
    std::string sFps = std::to_string(FPS);
    const char *fpsVal = sFps.c_str();
    surface = TTF_RenderText_Solid(sm_font, fpsVal, Gold);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    box.x = 2425;
    box.y = 50;
    box.w = 50;
    box.h = 50;
    SDL_RenderCopy(renderer, texture, NULL, &box);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);

    // Draw Nes Clock Speed Text
    const char *csText = "CLOCK SPEED";
    surface = TTF_RenderText_Solid(sm_font, csText, Red);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    box.x = 2325;
    box.y = 100;
    box.w = 70;
    box.h = 50;
    SDL_RenderCopy(renderer, texture, NULL, &box);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
    

    // Draw Nes Clock Speed 
    sFps = std::to_string((nesClockSpeed));
    const char *clockspeed = sFps.c_str();
    surface = TTF_RenderText_Solid(sm_font, clockspeed, Gold);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    box.x = 2425;
    box.y = 100;
    box.w = 90;
    box.h = 50;
    SDL_RenderCopy(renderer, texture, NULL, &box);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
    nesClockSpeed = 0.0;

    // Draw MARS Rendering Speed Text
    const char *rsText = "REND SPEED";
    surface = TTF_RenderText_Solid(sm_font, rsText, Red);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    box.x = 2325;
    box.y = 150;
    box.w = 70;
    box.h = 50;
    SDL_RenderCopy(renderer, texture, NULL, &box);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
    

    // Draw MARS Rendering Clock Speed 
    marsSpeed = SDL_GetTicks();
    marsSpeed = (marsSpeed - marsStart);
    marsStart = SDL_GetTicks();

    sFps = std::to_string((marsSpeed));
    const char *rendspeed = sFps.c_str();
    surface = TTF_RenderText_Solid(sm_font, rendspeed, Gold);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    box.x = 2425;
    box.y = 150;
    box.w = 90;
    box.h = 50;
    SDL_RenderCopy(renderer, texture, NULL, &box);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
    nesClockSpeed = 0.0;
}


inline void MARS::drawPixels()
{
    /*
    if (nes.ppu.frameComplete == true || debMode == true)
    {
        SDL_RenderSetLogicalSize(renderer, LOGICAL_WIDTH, LOGICAL_HEIGHT);
        //auto pix = nes.ppu.getScreenPixels();
        testCounter = 0;

        for (auto i = nes.ppu.screenPixels.begin(); i != nes.ppu.screenPixels.end(); i++)
        {
            // Set Color of Pixel Value
            SDL_SetRenderDrawColor(renderer, nes.ppu.screenPixels[testCounter].red, nes.ppu.screenPixels[testCounter].green, nes.ppu.screenPixels[testCounter].blue, 255);

            // Draw pixel on Coordinate Plane
            SDL_RenderDrawPoint(renderer, nes.ppu.screenPixels[testCounter].X, nes.ppu.screenPixels[testCounter].Y);
            testCounter++;
        };

        SDL_RenderPresent(renderer);
        SDL_RenderSetLogicalSize(renderer, WIDTH, HEIGHT);
        drawPatternTables();

        if (nes.ppu.frameComplete)
        {
            nes.ppu.frameComplete = false;
            nes.ppu.screenPixels.clear();
        };
    };
    */

    if (nes.ppu.frameComplete == true ) // || debMode == true
    {
        //pixelTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, LOGICAL_WIDTH, LOGICAL_HEIGHT);

        SDL_RenderSetLogicalSize(renderer, LOGICAL_WIDTH, LOGICAL_HEIGHT);
        int32_t pitch = 0;

        // This will hold a pointer to the memory position in VRAM where our Back Buffer texture lies
        uint32_t* pixelBuffer = nullptr;

        // Lock the memory in order to write our Back Buffer image to it
        if (!SDL_LockTexture(pixelTexture, NULL, (void**)&pixelBuffer, &pitch))
        {
            // The pitch of the Back Buffer texture in VRAM must be divided by four bytes
            // as it will always be a multiple of four
            pitch /= sizeof(uint32_t);

            // Fill texture with  pixels
            for (uint32_t i = 0; i < LOGICAL_WIDTH * LOGICAL_HEIGHT; i++)
            {
                pixelBuffer[i] = aRGB(nes.ppu.screenPixels[i].red, nes.ppu.screenPixels[i].green, nes.ppu.screenPixels[i].blue, 255);
            };
            // Unlock the texture in VRAM to let the GPU know we are done writing to it
            SDL_UnlockTexture(pixelTexture);
            SDL_RenderCopy(renderer, pixelTexture, NULL, NULL);

        };
        SDL_RenderSetLogicalSize(renderer, WIDTH, HEIGHT);
        if (nes.ppu.frameComplete)
        {
            nes.ppu.frameComplete = false;
            //SDL_DestroyTexture(pixelTexture);

        };

    };
    
}


uint32_t MARS::aRGB(uint32_t R, uint32_t G, uint32_t B, uint32_t a)
{
    uint32_t color = (a << 24) | (R << 16) | (G << 8) | B;
    return color;
}


inline void MARS::drawPatternTables()
{
    // Draw Left Plane Pattern Table
    uint32_t j = 0;

    uint32_t* leftPlaneBuffer = nullptr;
    uint32_t* rightPlaneBuffer = nullptr;
    int32_t location;
    int32_t pitch = 0;
    pat.texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 128, 128);

    // Lock the memory in order to write our Back Buffer image to it
    if (!SDL_LockTexture(pat.texture, NULL, (void**)&leftPlaneBuffer, &pitch))
    {
        pitch /= sizeof(uint32_t);
        j = 0;

        for (auto i = plane.left.begin(); i != plane.left.end(); i++)
        {
            if (plane.left[j].Y == 0)
            {
                if (plane.left[j].Y == 0 && plane.left[j].X == 0)
                {
                    location = 0;
                }
                else if (plane.left[j].Y == 0)
                {
                    location = plane.left[j].X;
                };
            }
            else
            {
                location =  (plane.left[j].Y * 128) + plane.left[j].X;
            };
            leftPlaneBuffer[location] = aRGB(plane.left[j].red, plane.left[j].green, plane.left[j].blue, 255);
            ++j;
        };
        pat.box.x = 2000;
        pat.box.y = 50;
        pat.box.w = 128;
        pat.box.h = 128;
        SDL_UnlockTexture(pat.texture);
        SDL_RenderCopy(renderer, pat.texture, NULL, &pat.box);
        SDL_DestroyTexture(pat.texture);
    };

    // Draw Right Plane Pattern Table
    pat.texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 128, 128);
    pitch = 0;
    if (!SDL_LockTexture(pat.texture, NULL, (void**)&rightPlaneBuffer, &pitch))
    {
        pitch /= sizeof(uint32_t);
        j = 0;

        for (auto i = plane.right.begin(); i != plane.right.end(); i++)
        {
            if (plane.right[j].Y == 0)
            {
                if (plane.right[j].Y == 0 && plane.right[j].X == 0)
                {
                    location = 0;
                }
                else if (plane.right[j].Y == 0)
                {
                    location = plane.right[j].X;
                };
            }
            else
            {
                location =  (plane.right[j].Y * 128) + plane.right[j].X;
            };
            leftPlaneBuffer[location] = aRGB(plane.right[j].red, plane.right[j].green, plane.right[j].blue, 255);
            ++j;
        };
        pat.box.x = 2150;
        pat.box.y = 50;
        pat.box.w = 128;
        pat.box.h = 128;
        SDL_UnlockTexture(pat.texture);
        SDL_RenderCopy(renderer, pat.texture, NULL, &pat.box);
        SDL_DestroyTexture(pat.texture);
    };

}

void MARS::resetPatternTables(uint8_t palette)
{
    //palette &= 0x03;
    plane.left = nes.ppu.getPatternTables(0, palette);
    nes.ppu.patterns.clear();

    plane.right = nes.ppu.getPatternTables(1, palette);
    nes.ppu.patterns.clear();
}

inline void MARS::drawPalettes()
{
    auto palettes = nes.ppu.getPalettes();

    int i = 0;
    X = RIGHT_L_JUSTIFIED;
    Y = 200;
    W = 20;
    H = 20;
    int indx = 0;

    for (auto j = palettes.begin(); j != palettes.end(); j++)
    {
        if(indx == 4 ||  indx == 12 || indx == 20 ||  indx == 28)
        {
            Y = 220;
            X = X - 80;
        };

        if (indx == 8  || indx == 16 || indx == 24 || indx == 32)
        {
            Y = 200;
            X = X + 40;
        };

        SDL_SetRenderDrawColor(renderer, palettes[i].red, palettes[i].green, palettes[i].blue, 255);
        box.x = X;
        box.y = Y;
        box.w = W;
        box.h = H;
        SDL_RenderFillRect(renderer, &box);

        X += 20;

        i++;
        indx++;

    }
}

void MARS::logWrite()
{
    std::string disassembly = nes.cpu.thisDis;
    ofstream wLog("C:\\Users\\Thugs4Less\\Desktop\\Program Projects\\NES\\log\\disassembly.txt", std::ofstream::app);

    if(nes.cpu.newInstruction == true && ((nes.NES_SystemClock % 3) == 0))
    {       
        if(wLog.is_open())
        {
            wLog << disassembly << "\t";
            wLog << std::hex << (int)nes.cpu.d.get_opcode;
            wLog << " ";
            wLog << nes.cpu.d.get_opcode_instruction[0];
            wLog << " ";
            wLog << nes.cpu.d.get_opcode_instruction[1];

            wLog << "\t\t A:";  wLog << std::hex << (int) nes.cpu.reg.A;
            wLog << " X:";    wLog << std::hex << (int) nes.cpu.reg.X;
            wLog << " Y:";    wLog << std::hex << (int) nes.cpu.reg.Y;
            wLog << " P:";    wLog << std::hex << (int) nes.cpu.reg.P;
            wLog << " SP:";   wLog << std::hex << (int) nes.cpu.reg.S;

            //wLog << "\t R/W:";  wLog << nes.bus_accessType;
            wLog << "\t\t Controller:";  wLog << std::hex << (int) nes.controllers[0];
            //wLog << "\t\t Controller State:";  wLog << std::hex << (int) nes.controllerState[0];


            //wLog << "\t\t Bus Data:";  wLog << std::hex << (int) nes.bus_data;

            wLog << "\n";
            wLog.close();
        };
    };
    
}

void MARS::drawScreen()
{
    if(nes.ppu.frameComplete == true || debMode == true)
    {
        SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);
        SDL_RenderClear(renderer);
                
        drawPixels();
        drawCpu();
        drawPpu();
        drawPalettes();
        drawPatternTables();
        drawSystemComponents();

        SDL_RenderPresent(renderer);
    };
}


bool MARS::eventHandler()
{  
    bool On = true;
    debMode = false;

    nesClockStart = SDL_GetTicks();
    while (On)
    {
        nes.systemClock();
        //logWrite();

        if(nes.ppu.frameComplete == true)
        {
            ticks = SDL_GetTicks();
            marsStart = SDL_GetTicks();
            nesClockSpeed = (double)(ticks - nesClockStart);

            if (SDL_PollEvent(&event))
            {
                switch(event.type)
                {
                    case SDL_QUIT:
                        On = false;

                    case SDL_KEYDOWN:
                        keyboardInput(event);
                        break;
                    
                    default:
                        break;
                };
            };

            ticks = SDL_GetTicks();

            if ((ticks - prevTick) > 1000) 
            {
                prevTick = ticks;
                FPS = count;
                count = 0;
            }
            else
            {
                count++;
            };
            getControllerState();
            drawScreen();
            nesClockStart = SDL_GetTicks();
        };
    };
    SDL_DestroyRenderer(renderer);
    SDL_DestroyRenderer(rend_debug);
    return false;
}

bool MARS::debugMode()
{
    debMode = true;

    while (SDL_WaitEvent(&event))
    {
        switch(event.type)
        {
            case SDL_QUIT:
                SDL_Quit();
                return false;

            case SDL_KEYDOWN:
                keyboardInput(event);
                break;
                
            default:
                break;
        };
    };
    return true;
}

void MARS::init()
{
    marsStart = SDL_GetTicks();

    // Initialize SDL 
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();
    
    // Main window & Debug window
    window = SDL_CreateWindow("MARS", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);

    // Main Renderer & Debug Renderer
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    pixelTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, LOGICAL_WIDTH, LOGICAL_HEIGHT);

    // Set Font Elements
    lg_font = TTF_OpenFont("fonts/Anonymous.ttf", 80);
    TTF_SetFontStyle(lg_font, TTF_STYLE_BOLD);

    sm_font = TTF_OpenFont("fonts/Anonymous.ttf", 80);

    uint8_t console = 1;
    switch (console)
    {
        case 1:
            cart = std::make_shared<Cartridge>("roms/nestest.nes");
            if (!cart->ImageValid())
            {
			    SDL_Quit();
            };
            selectedGame = "NES Test";
            break;
            break;

        case 2:
            cart = std::make_shared<Cartridge>("roms/donkeykong.nes");
            if (!cart->ImageValid())
            {
			    SDL_Quit();
            };
            selectedGame = "Donkey Kong";
            break;

        case 3:
            cart = std::make_shared<Cartridge>("roms/iceclimber.nes");
            if (!cart->ImageValid())
            {
			    SDL_Quit();
            };
            selectedGame = "Ice Climber";
            break;

        case 4:
            cart = std::make_shared<Cartridge>("roms/Super Mario Bros. (Japan, USA).nes");
            if (!cart->ImageValid())
            {
			    SDL_Quit();
            };
            selectedGame = "Super Mario Bros";
            break;
        
        default:
            break;
    };

    nes.insertCartridge(cart);
    nes.systemReset();
    resetPatternTables(0);
}


int main(int argc, char* argv[])
{
    MARS mars;

    //mars.debugMode();       // Debugger Event Handler
    mars.eventHandler();    // Main Event Handler
    SDL_Quit();
}









//nes.cpuRam[0xFFFC] = 0x00;
//nes.cpuRam[0xFFFD] = 0xC0;

// COMPILE
//g++ *.cpp -IC:\SDL_32bit\i686-w64-mingw32\include\SDL2 -IC:\SDL_ttf\include\SDL2 -LC:\SDL_32bit\i686-w64-mingw32\lib -LC:\SDL_ttf\lib -w -Wl,-subsystem,windows -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf -o mars.exe

// g++ *.cpp -IC:\SDL_32bit\i686-w64-mingw32\include\SDL2 -IC:\SDL_ttf\include\SDL2 -LC:\SDL_32bit\i686-w64-mingw32\lib -LC:\SDL_ttf\lib -w -Wl,-subsystem,windows -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf -o mars.exe