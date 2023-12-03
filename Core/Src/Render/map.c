#include "render/map.h"

static int rawMap0[] = {
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,0,0,1,1,0,1,0,0,0,0,1,
	1,0,0,0,0,0,1,0,1,1,0,1,1,0,1,0,0,1,0,1,
	1,0,0,0,0,0,1,0,1,1,0,1,1,0,1,0,0,0,0,1,
	1,0,0,0,0,0,1,0,1,1,0,0,0,0,1,0,0,0,0,1,
	1,0,0,0,0,0,1,0,1,1,1,0,1,0,1,1,1,1,1,1,
	1,0,0,0,0,1,1,0,1,0,1,0,1,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,1,0,1,0,1,1,1,1,1,1,0,1,
	1,0,0,0,0,1,1,1,1,0,1,0,0,0,0,0,0,1,0,2,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
};

static int rawMap1[] = {
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,0,0,1,1,1,1,1,0,1,0,1,0,0,0,0,0,1,1,1,
	1,0,0,0,0,0,0,1,0,1,0,1,0,0,0,0,0,1,0,1,
	1,0,0,1,1,1,0,1,0,1,0,1,0,0,0,0,0,1,0,1,
	1,0,0,0,0,1,0,1,0,1,0,1,1,1,1,0,1,1,0,1,
	1,0,0,1,0,1,0,1,0,1,0,1,0,0,0,0,0,0,0,1,
	1,0,0,1,0,1,0,1,0,1,0,1,2,1,1,1,1,1,1,1,
	1,0,0,1,0,1,0,1,0,1,0,0,1,0,0,0,0,0,0,1,
	1,0,0,1,0,1,0,1,0,1,0,0,1,1,1,1,1,1,0,1,
	1,0,0,1,0,0,0,1,0,1,0,0,0,0,0,0,0,0,0,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
};


static int rawMap2[] = {
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,
	1,0,0,0,1,1,1,1,1,1,1,1,1,0,1,0,1,0,1,1,
	1,0,1,0,1,0,0,0,1,0,1,0,1,0,1,0,0,0,0,1,
	1,0,1,0,1,1,1,0,1,0,1,0,1,0,1,1,1,0,0,1,
	1,0,1,0,0,0,0,0,1,0,0,0,1,0,1,1,1,1,0,1,
	1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,1,1,1,1,0,1,0,1,0,1,0,1,0,1,1,1,1,1,1,
	1,0,0,0,1,0,1,1,1,1,1,1,1,1,1,1,0,0,0,1,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,
	1,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
};

static int* maps[] = { rawMap0, rawMap1, rawMap2 };
static int mapIndex = 0;

/**
  * @brief  It changes the current map set in the Map structure passed as a parameter with one the available in the map.c file
  * @param  m : The Map structure that we want to change
  */
void changeMap(Map *m)
{
	m->map = maps[mapIndex++];
	mapIndex = mapIndex == 3 ? 0 : mapIndex;
}
