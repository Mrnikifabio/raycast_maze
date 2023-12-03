/*
 * render.h
 *
 *  Created on: 5 dic 2022
 *      Author: fabio
 */

#ifndef INC_RENDER_RENDER_H_
#define INC_RENDER_RENDER_H_


#include "FreeRTOS.h"
#include "semphr.h"
#include "render/screen.h"
#include "render/map.h"
#include <stdbool.h>
#include <math.h>

//The map will be 1/5 of the total available area of the display
#define MAP_SCALE 5
//PI Helpers
#define P2 M_PI/2
#define P3 3*M_PI/2
//math.h functions use radians rather than degrees therefore here what a degree corresponds to in radians
#define DEGREE_RADIAN 0.0174533
//the field of view of the player in degrees, which literally translates to the number of ray that will be casted
#define FOV 62



typedef struct {
	float x;
	float y;
} vec2;


typedef struct {
	int index;
	float distance;
	vec2 pos;
	float angle;
	bool vertical;
} Ray;



void castRays(float focalX, float focalY, float focalAngle, Map *m);
void drawControls(Screen *s, Map *m, int scale);
void drawMapRays(float focalX, float focalY);
void drawRays(Map *m, Screen *s, float focalAngle);
void drawBackground(Screen *s);
void drawMap(Map *m, Screen *s);

#endif /* INC_RENDER_RENDER_H_ */
