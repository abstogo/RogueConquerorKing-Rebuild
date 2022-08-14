/*
 * libtcod CPP samples
 * This code demonstrates various usages of libtcod modules
 * It's in the public domain.
 */

// uncomment this to disable SDL sample (might cause compilation issues on some systems)
//#define NO_SDL_SAMPLE

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stack>
#include "libtcod.hpp"
#define SDL_MAIN_HANDLED
//#include <SDL.h>
#include <vector>
#include <list>
#include <queue>
#include "Maps.h"
#include "Character.h"
#include "Class.h"
#include "Game.h"
#include "OutputLog.h"

// sample screen position


// ***************************
// samples rendering functions
// ***************************


// ***************************
// keeping the offscreen console sample to use as a dialog system later
// ***************************
/**
void render_offscreen(bool first, TCOD_key_t*key, TCOD_mouse_t *mouse) {
	static TCODConsole secondary(SAMPLE_SCREEN_WIDTH/2,SAMPLE_SCREEN_HEIGHT); // second screen
	static TCODConsole screenshot(SAMPLE_SCREEN_WIDTH,SAMPLE_SCREEN_HEIGHT); // second screen
	static bool init=false; // draw the secondary screen only the first time
	static int counter=0;
	static int x=0,y=0; // secondary screen position
	static int xdir=1,ydir=1; // movement direction
	if (! init ) {
		init=true;
		secondary.printFrame(0,0,SAMPLE_SCREEN_WIDTH/2,SAMPLE_SCREEN_HEIGHT,false,TCOD_BKGND_SET,"Offscreen console");
		secondary.printRectEx(SAMPLE_SCREEN_WIDTH/4,2,SAMPLE_SCREEN_WIDTH-2,SAMPLE_SCREEN_HEIGHT,
			TCOD_BKGND_NONE,TCOD_CENTER,"You can render to an offscreen console and blit in on another one, simulating alpha transparency.");
	}
	if ( first ) {
		TCODSystem::setFps(30); // fps limited to 30
		// get a "screenshot" of the current sample screen
		TCODConsole::blit(&sampleConsole,0,0,SAMPLE_SCREEN_WIDTH,SAMPLE_SCREEN_HEIGHT,
							&screenshot,0,0);
	}
	counter++;
	if ( counter % 20 == 0 ) {
		// move the secondary screen every 2 seconds
		x+=xdir;y+=ydir;
		if ( x == SAMPLE_SCREEN_WIDTH/2+5 ) xdir=-1;
		else if ( x == -5 ) xdir=1;
		if ( y == SAMPLE_SCREEN_HEIGHT/2+5 ) ydir=-1;
		else if ( y == -5 ) ydir=1;
	}
	// restore the initial screen
	TCODConsole::blit(&screenshot,0,0,SAMPLE_SCREEN_WIDTH,SAMPLE_SCREEN_HEIGHT,
					&sampleConsole,0,0);
	// blit the overlapping screen
	TCODConsole::blit(&secondary,0,0,SAMPLE_SCREEN_WIDTH/2,SAMPLE_SCREEN_HEIGHT/2,
					&sampleConsole,x,y,1.0f,0.75f);

}
*/

// ***************************
// Use bsp sample as basis for lair/dungeon maps
// ***************************

static int bspDepth=8;
static int minRoomSize=4;
static bool randomRoom=false; // a room fills a random part of the node or the maximum available space ?
static bool roomWalls=true; // if true, there is always a wall on north & west side of a room
typedef	char map_t[SAMPLE_SCREEN_WIDTH][SAMPLE_SCREEN_HEIGHT];

// draw a vertical line
void vline(map_t *map,int x, int y1, int y2) {
	int y=y1;
	int dy=(y1>y2?-1:1);
	(*map)[x][y]=' ';
	if ( y1 == y2 ) return;
	do {
		y+=dy;
		(*map)[x][y]=' ';
	} while (y!=y2);
}


// draw a vertical line up until we reach an empty space
void vline_up(map_t *map,int x, int y) {
	while (y >= 0 && (*map)[x][y] != ' ') {
		(*map)[x][y]=' ';
		y--;
	}
}

// draw a vertical line down until we reach an empty space
void vline_down(map_t *map,int x, int y) {
	while (y < SAMPLE_SCREEN_HEIGHT && (*map)[x][y] != ' ') {
		(*map)[x][y]=' ';
		y++;
	}
}

// draw a horizontal line
void hline(map_t *map,int x1, int y, int x2) {
	int x=x1;
	int dx=(x1>x2?-1:1);
	(*map)[x][y]=' ';
	if ( x1 == x2 ) return;
	do {
		x+=dx;
		(*map)[x][y]=' ';
	} while (x!=x2);
}

// draw a horizontal line left until we reach an empty space
void hline_left(map_t *map,int x, int y) {
	while (x >= 0 && (*map)[x][y] != ' ') {
		(*map)[x][y]=' ';
		x--;
	}
}

// draw a horizontal line right until we reach an empty space
void hline_right(map_t *map,int x, int y) {
	while (x < SAMPLE_SCREEN_WIDTH && (*map)[x][y] != ' ') {
		(*map)[x][y]=' ';
		x++;
	}
}

// the class building the dungeon from the bsp nodes
//#include <stdio.h>
class BspListener : public ITCODBspCallback {
public :
	bool visitNode(TCODBsp *node, void *userData) {
		map_t *map=(map_t *)userData;
		if ( node->isLeaf() ) {
			// calculate the room size
			int minx = node->x+1;
			int maxx = node->x+node->w-1;
			int miny = node->y+1;
			int maxy = node->y+node->h-1;
			if (! roomWalls ) {
				if ( minx > 1 ) minx--;
				if ( miny > 1 ) miny--;
			}
			if (maxx == SAMPLE_SCREEN_WIDTH-1 ) maxx--;
			if (maxy == SAMPLE_SCREEN_HEIGHT-1 ) maxy--;
			if ( randomRoom ) {
				minx = TCODRandom::getInstance()->getInt(minx,maxx-minRoomSize+1);
				miny = TCODRandom::getInstance()->getInt(miny,maxy-minRoomSize+1);
				maxx = TCODRandom::getInstance()->getInt(minx+minRoomSize-1,maxx);
				maxy = TCODRandom::getInstance()->getInt(miny+minRoomSize-1,maxy);
			}
			// resize the node to fit the room
//printf("node %dx%d %dx%d => room %dx%d %dx%d\n",node->x,node->y,node->w,node->h,minx,miny,maxx-minx+1,maxy-miny+1);
			node->x=minx;
			node->y=miny;
			node->w=maxx-minx+1;
			node->h=maxy-miny+1;
			// dig the room
			for (int x=minx; x <= maxx; x++ ) {
				for (int y=miny; y <= maxy; y++ ) {
					(*map)[x][y]=' ';
				}
			}
		} else {
//printf("lvl %d %dx%d %dx%d\n",node->level, node->x,node->y,node->w,node->h);
			// resize the node to fit its sons
			TCODBsp *left=node->getLeft();
			TCODBsp *right=node->getRight();
			node->x=MIN(left->x,right->x);
			node->y=MIN(left->y,right->y);
			node->w=MAX(left->x+left->w,right->x+right->w)-node->x;
			node->h=MAX(left->y+left->h,right->y+right->h)-node->y;
			// create a corridor between the two lower nodes
			if (node->horizontal) {
				// vertical corridor
				if ( left->x+left->w -1 < right->x || right->x+right->w-1 < left->x ) {
					// no overlapping zone. we need a Z shaped corridor
					int x1=TCODRandom::getInstance()->getInt(left->x,left->x+left->w-1);
					int x2=TCODRandom::getInstance()->getInt(right->x,right->x+right->w-1);
					int y=TCODRandom::getInstance()->getInt(left->y+left->h,right->y);
					vline_up(map,x1,y-1);
					hline(map,x1,y,x2);
					vline_down(map,x2,y+1);
				} else {
					// straight vertical corridor
					int minx=MAX(left->x,right->x);
					int maxx=MIN(left->x+left->w-1,right->x+right->w-1);
					int x=TCODRandom::getInstance()->getInt(minx,maxx);
					vline_down(map,x,right->y);
					vline_up(map,x,right->y-1);
				}
			} else {
				// horizontal corridor
				if ( left->y+left->h -1 < right->y || right->y+right->h-1 < left->y ) {
					// no overlapping zone. we need a Z shaped corridor
					int y1=TCODRandom::getInstance()->getInt(left->y,left->y+left->h-1);
					int y2=TCODRandom::getInstance()->getInt(right->y,right->y+right->h-1);
					int x=TCODRandom::getInstance()->getInt(left->x+left->w,right->x);
					hline_left(map,x-1,y1);
					vline(map,x,y1,y2);
					hline_right(map,x+1,y2);
				} else {
					// straight horizontal corridor
					int miny=MAX(left->y,right->y);
					int maxy=MIN(left->y+left->h-1,right->y+right->h-1);
					int y=TCODRandom::getInstance()->getInt(miny,maxy);
					hline_left(map,right->x-1,y);
					hline_right(map,right->x,y);
				}
			}
		}
		return true;
	}
};
/**
void render_bsp(bool first, TCOD_key_t*key, TCOD_mouse_t *mouse) {
	static TCODBsp *bsp=NULL;
	static bool generate=true;
	static bool refresh=false;
	static map_t map;
	static TCODColor darkWall(0,0,100);
	static TCODColor darkGround(50,50,150);
	static BspListener listener;
	if ( generate || refresh ) {
		// dungeon generation
		if (! bsp ) {
			// create the bsp
			bsp = new TCODBsp(0,0,SAMPLE_SCREEN_WIDTH,SAMPLE_SCREEN_HEIGHT);
		} else {
			// restore the nodes size
			bsp->resize(0,0,SAMPLE_SCREEN_WIDTH,SAMPLE_SCREEN_HEIGHT);
		}
		memset(map,'#',sizeof(char)*SAMPLE_SCREEN_WIDTH*SAMPLE_SCREEN_HEIGHT);
		if ( generate ) {
			// build a new random bsp tree
			bsp->removeSons();
			bsp->splitRecursive(NULL,bspDepth,minRoomSize+(roomWalls?1:0),minRoomSize+(roomWalls?1:0),1.5f,1.5f);
		}
		// create the dungeon from the bsp
		bsp->traverseInvertedLevelOrder(&listener,&map);
		generate=false;
		refresh=false;
	}
	sampleConsole.clear();
	sampleConsole.setDefaultForeground(TCODColor::white);
	sampleConsole.print(1,1,"ENTER : rebuild bsp\nSPACE : rebuild dungeon\n+-: bsp depth %d\n* /: room size %d\n1 : random room size %s",
		bspDepth,minRoomSize,
		randomRoom ? "ON" : "OFF");
	if ( randomRoom )
	sampleConsole.print(1,6,"2 : room walls %s",
		roomWalls ? "ON" : "OFF"	);
	// render the level
	for (int y=0; y < SAMPLE_SCREEN_HEIGHT; y++ ) {
		for (int x=0; x < SAMPLE_SCREEN_WIDTH; x++ ) {
			bool wall= ( map[x][y] == '#' );
			sampleConsole.setCharBackground(x,y,wall ? darkWall : darkGround, TCOD_BKGND_SET );
		}
	}
	if ( key->vk == TCODK_ENTER || key->vk == TCODK_KPENTER ) {
		generate=true;
	} else if (key->c==' ') {
		refresh=true;
	} else if (key->c=='+') {
		bspDepth++;
		generate=true;
	} else if (key->c=='-' && bspDepth > 1) {
		bspDepth--;
		generate=true;
	} else if (key->c=='*') {
		minRoomSize++;
		generate=true;
	} else if (key->c=='/' && minRoomSize > 2) {
		minRoomSize--;
		generate=true;
	} else if (key->c=='1' || key->vk == TCODK_1 || key->vk == TCODK_KP1) {
		randomRoom=!randomRoom;
		if (! randomRoom ) roomWalls=true;
		refresh=true;
	} else if (key->c=='2' || key->vk == TCODK_2 || key->vk == TCODK_KP2) {
		roomWalls=!roomWalls;
		refresh=true;
	}
}
*/

// ***************************
// the main function
// ***************************
int main( int argc, char *argv[] ) {

	gGame = new Game();
	
	bool first=true; // first time we render a sample
	TCOD_key_t key = {TCODK_NONE,0};
	TCOD_mouse_t mouse;
	
	const char *font="data/fonts/consolas10x10_gs_tc.png";
	
	int nbCharHoriz=0,nbCharVertic=0;
	int argn;
	int fullscreenWidth=0;
	int fullscreenHeight=0;
	TCOD_renderer_t renderer=TCOD_RENDERER_SDL2;
	//TCOD_renderer_t renderer = TCOD_RENDERER_OPENGL;
	bool fullscreen=false;
	int fontFlags=TCOD_FONT_TYPE_GREYSCALE|TCOD_FONT_LAYOUT_TCOD, fontNewFlags=0;
	
	// initialize the root console (open the game window)
	for (argn=1; argn < argc; argn++) {
		if ( strcmp(argv[argn],"-font") == 0 && argn+1 < argc) {
			argn++;
			font=argv[argn];
			fontFlags=0;
		} else if ( strcmp(argv[argn],"-font-nb-char") == 0 && argn+2 < argc ) {
			argn++;
			nbCharHoriz=atoi(argv[argn]);
			argn++;
			nbCharVertic=atoi(argv[argn]);
			fontFlags=0;
		} else if ( strcmp(argv[argn],"-fullscreen-resolution") == 0 && argn+2 < argc ) {
			argn++;
			fullscreenWidth=atoi(argv[argn]);
			argn++;
			fullscreenHeight=atoi(argv[argn]);
		} else if ( strcmp(argv[argn],"-renderer") == 0 && argn+1 < argc ) {
			argn++;
			renderer=(TCOD_renderer_t)atoi(argv[argn]);
		} else if ( strcmp(argv[argn],"-fullscreen") == 0 ) {
			fullscreen=true;
		} else if ( strcmp(argv[argn],"-font-in-row") == 0 ) {
			fontNewFlags |= TCOD_FONT_LAYOUT_ASCII_INROW;
			fontFlags=0;
		} else if ( strcmp(argv[argn],"-font-greyscale") == 0 ) {
			fontNewFlags |= TCOD_FONT_TYPE_GREYSCALE;
			fontFlags=0;
		} else if ( strcmp(argv[argn],"-font-tcod") == 0 ) {
			fontNewFlags |= TCOD_FONT_LAYOUT_TCOD;
			fontFlags=0;
		} else if ( strcmp(argv[argn],"-help") == 0 || strcmp(argv[argn],"-?") == 0) {
			printf ("options :\n");
			printf ("-font <filename> : use a custom font\n");
			printf ("-font-nb-char <nb_char_horiz> <nb_char_vertic> : number of characters in the font\n");
			printf ("-font-in-row : the font layout is in row instead of columns\n");
			printf ("-font-tcod : the font uses TCOD layout instead of ASCII\n");
			printf ("-font-greyscale : antialiased font using greyscale bitmap\n");
			printf ("-fullscreen : start in fullscreen\n");
			printf ("-fullscreen-resolution <screen_width> <screen_height> : force fullscreen resolution\n");
			printf ("-renderer <num> : set renderer. 0 : GLSL 1 : OPENGL 2 : SDL\n");
			exit(0);
		} else {
			// ignore parameter
		}
	}

	if ( fontFlags == 0 ) fontFlags=fontNewFlags;
	TCODConsole::setCustomFont(font,fontFlags,nbCharHoriz,nbCharVertic);
	if ( fullscreenWidth > 0 ) {
		TCODSystem::forceFullscreenResolution(fullscreenWidth,fullscreenHeight);
	}
	
	TCODConsole::initRoot(80,50,"libtcod C++ sample",fullscreen,renderer);
	gLog = new OutputLog();
	gGame->StartGame();
	gGame->MainLoop();
	return 0;
}
