// **************************************************
// MazeDemo by t33buSoft 2023
// **************************************************
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include "neslib.h"
#include "maze.h"

#pragma bss-name (push,"ZEROPAGE")
uint8_t oam_off;
uint8_t maze_stackptr;
uint8_t vram_buf[4] = { NT_UPD_EOF, NT_UPD_EOF, NT_UPD_EOF, NT_UPD_EOF };
#pragma bss-name (pop)

void vram_buffer_clear(void);
void vram_update_tile(uint8_t row, uint8_t col, uint8_t data);
void maze_draw(void);
bool maze_search(uint8_t row, uint8_t col);
void sprite_draw(void);
	
// **************************************************
// Graphics
// **************************************************
const char ATTRIBUTE_TABLE[0x40] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
};

const char PALETTE[32] = { 
  0x0e,0x00,0x20,0x30, 
	0x0e,0x0a,0x09,0x29,
	0x00,0x00,0x10,0x20,
	0x00,0x06,0x16,0x26,
	0x0F,0x1C,0x2B,0x36, 
	0x0f,0x06,0x15,0x10, 
	0x0c,0x14,0x23,0x37,
	0x0c,0x14,0x23,0x37
};

#define DELAY					25	// frames
#define SPRITE_LEN		2		// how many sprites
#define TILE_OFFSET		7		// screen offset to draw
#define	TILE_PLAYER		0x0	// tileset
#define TILE_PATH			0x1
#define TILE_EXIT			0x2
#define TILE_WALL			0x3
#define	TILE_VISITED	0x4

const char maze_title1[] = " MazeDemo ";
const char maze_title2[] = " t33busoft";
const char maze_title3[] = "      2023";

struct sprite_t {
	uint8_t x;
	uint8_t y;
	uint8_t tile;
	uint8_t attrib;	
} sprites[SPRITE_LEN];

// **************************************************
void vram_buffer_clear(void) {
	vram_buf[0]=NT_UPD_EOF;	
}

// **************************************************
void vram_update_tile(uint8_t row, uint8_t col, uint8_t data) {
	vram_buf[0] = MSB(NTADR_A(col,row));
	vram_buf[1] = LSB(NTADR_A(col,row));
	vram_buf[2] = data;
  vram_buf[3] = NT_UPD_EOF; 
	set_vram_update(vram_buf);
  ppu_wait_frame();
  vram_buffer_clear();		
}

// **************************************************
void sprite_draw() {
	uint8_t i=0;
	uint8_t oam_id = 0;
	for (i=0; i < SPRITE_LEN; i++) {
		oam_id = oam_spr(sprites[i].x,sprites[i].y,
						 sprites[i].tile, sprites[i].attrib, oam_id);
		ppu_wait_frame();
	}
}

// **************************************************
void init_sprites() {	
	// player sprite
	sprites[0].x = 0;
	sprites[0].y = 0;
	sprites[0].tile = TILE_PLAYER;
	sprites[0].attrib = 0;

	// exit sprite
	sprites[1].x = 64;
	sprites[1].y = 119;
	sprites[1].tile = TILE_EXIT;
	sprites[1].attrib = 1;	
}
// **************************************************
// Main function
// **************************************************
void main(void) {
	
	bool result = false;
	
  // init background
	oam_clear();
  pal_all(PALETTE);
  vram_adr(NAMETABLE_A);	
  vram_fill(TILE_WALL, 32*30);
  vram_write(ATTRIBUTE_TABLE, sizeof(ATTRIBUTE_TABLE));
  vram_adr(NTADR_A(7,6));	
  vram_write(maze_title1,10);	
  vram_adr(NTADR_A(7,17));	
  vram_write(maze_title2,10);
  vram_adr(NTADR_A(7,18));	
	vram_write(maze_title3,10);	
	maze_draw();
  vram_buffer_clear();
  set_vram_update(vram_buf);	
  ppu_on_all();
	
	init_sprites();
	
	// run demo
	result = maze_search(1,1);
	
	// wait forever..
  while (1) {
	}
}

// **************************************************
// Maze 
// **************************************************

// stack for non-recursive search algorithm
uint16_t maze_stack[50];

// **************************************************
void maze_draw() {
	uint8_t i=0, j=0;
	for (i=1; i < 9; i++) {
		for (j=1; j < 9; j++) {						
			vram_adr(NTADR_A(i+TILE_OFFSET,j+TILE_OFFSET));	
			vram_put(maze[j][i]);					
		} 
	} 
} 

// **************************************************
bool maze_search(uint8_t row, uint8_t col) {
		
	// push first cell into stack
	maze_stackptr = 0;
	maze_stack[maze_stackptr++] = (row * 32) + col;
	
	// go through all cells in stack
	while (maze_stackptr != 0) {
		
		// pop top cell
		maze_stackptr--;
		row = maze_stack[maze_stackptr] / 32;
		col = maze_stack[maze_stackptr] % 32;
				
		// wait few frames for visual
		delay(DELAY);	

		// draw sprites
		sprites[0].x = (col+TILE_OFFSET)*8;
		sprites[0].y = ((row+TILE_OFFSET)*8)-1;		
		sprite_draw();				

		// check if exit found
		if (maze[row][col] == TILE_EXIT) {			
			maze_stackptr = 0;
			return true;
		// otherwise add neighborss to stack
		} else {					
			if (maze[row-1][col] < TILE_WALL)
				maze_stack[maze_stackptr++] = ((row-1) * 32) + col;	
			if (maze[row][col-1] < TILE_WALL)
				maze_stack[maze_stackptr++] = (row * 32) + (col-1);	
			if (maze[row+1][col] < TILE_WALL)
				maze_stack[maze_stackptr++] = ((row+1) * 32) + col;	
			if (maze[row][col+1] < TILE_WALL)
				maze_stack[maze_stackptr++] = (row * 32) + (col+1);	
			
			// mark current cell visited
			maze[row][col] = TILE_VISITED;			
			// update maze in screen
			vram_update_tile(row+TILE_OFFSET,col+TILE_OFFSET,TILE_VISITED);	
		}
	}
	return false;		
}


