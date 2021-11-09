OUT = Nes
CXX = g++
SDL = -IC:\SDL_32bit\i686-w64-mingw32\include\SDL2 -IC:\SDL_ttf\include\SDL2 -LC:\SDL_32bit\i686-w64-mingw32\lib -LC:\SDL_ttf\lib -Wl,-subsystem,windows -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf

OBJECTS = MARSengine.o Bus.o Cpu.o Cartridge.o Ppu.o Mapper.o Mapper_000.o

all: $(OUT)
$(OUT): $(OBJECTS)
	$(CXX) -o $@ $^ ${SDL}

$(OBJECTS): MARSengine.h Bus.h Cpu.h Cartridge.h Ppu.h Mapper.h Mapper_000.h

clean:
	del -f *.o


