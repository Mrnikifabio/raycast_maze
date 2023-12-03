/*
 * map.h
 *
 *  Created on: 5 dic 2022
 *      Author: fabio
 */

#ifndef INC_RENDER_MAP_H_
#define INC_RENDER_MAP_H_


typedef struct {
	int mapBlockX; //number of blocks in a row of the map
	int mapBlockY; //the number of row in a map
	int blockSize; //the width and height of a block in pixel
	int *map; //the actual map
} Map;


void changeMap(Map *m);

#endif /*INC_RENDER_MAP_H_*/
