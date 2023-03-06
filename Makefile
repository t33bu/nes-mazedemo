#
# Author: T33buSoft 
#
TARGET = mazedemo
TILES = tileset
CFG = $(TARGET).cfg
SRC = src
INCLUDES = include
LIB=lib
RES = res
CC65 = cc65
CA65 = ca65
LD65 = ld65
EMULATOR = d:\mesen\mesen

default: $(TARGET).nes

$(TARGET).nes: $(TARGET).o $(TILES).o $(RES)\$(CFG)
	$(LD65) -C $(RES)\$(CFG) -o $(TARGET).nes $(LIB)\crt0.o $(TILES).o $(TARGET).o $(LIB)\nes.lib --dbgfile $(TARGET).dbg
	del *.o
	@echo $(TARGET).nes created

$(TILES).o: $(SRC)\load_chars.s $(RES)\game.chr
	$(CA65) -o .\$(TILES).o $(SRC)\load_chars.s
	
$(TARGET).o: $(TARGET).s 
	$(CA65) -o .\$(TARGET).o $(SRC)\$(TARGET).s -g

$(TARGET).s: $(SRC)\$(TARGET).c $(INCLUDES)\maze.h
	$(CC65) -I $(INCLUDES) -Oirs $(SRC)\$(TARGET).c --add-source

run:
	$(EMULATOR) $(TARGET).nes

clean:
	del *.o
	del *.s
	del $(TARGET).nes
	del $(TARGET).dbg
	
	
	

