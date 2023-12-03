/*
 * render.c
 *
 *  Created on: 5 dic 2022
 *      Author: fabio
 */

#include "render/render.h"
#include "stm32f769i_discovery_lcd.h"
#include <math.h>

//global ray index number generator
static int rayIndex = 0;

//global array used to store the casted rays
static Ray rays[FOV];

static float distance(float ax, float ay, float bx, float by);
static void drawRayMap(float focalX, float focalY, Ray *r);
static void drawColumn(float focalAngle, Ray *r, Map *m, Screen *s);

/**
  * @brief  It calculates the length of a line given the coordinates of it starting and ending point
  * @note 	Its just pitagora on steroids
  * @param  ax : The starting x coordinate
  * @param  ay : The starting y coordinate
  * @param  bx : The ending x coordinate
  * @param  by : The ending y coordinate
  * @return the length of the line
  */
static float distance(float ax, float ay, float bx, float by)
{
	return sqrt((bx-ax)*(bx-ax) + (by-ay)*(by-ay));
}

/**
  * @brief  Draws the 2D map on the screen with the scale defined in the render.h
  * @param  s : The Screen used to display the game
  * @param  m : The map currently active in the game
  */
void drawMap(Map *m, Screen *s)
{
	int yInc = s->height/(m->mapBlockY*MAP_SCALE);
	int xInc = s->width/(m->mapBlockX*MAP_SCALE);

		for(int y = 0; y < m->mapBlockY; y++)
			for(int x = 0; x < m->mapBlockX; x++)
				switch(m->map[y*m->mapBlockX+x])
				{
				case 1:
					BSP_LCD_SetTextColor(LCD_COLOR_GREEN);
					BSP_LCD_FillRect(x*xInc, y*yInc, xInc, yInc);
					BSP_LCD_SetTextColor(LCD_COLOR_BLUE);
					BSP_LCD_DrawRect(x*xInc, y*yInc, xInc, yInc);
				break;
				case 0:
					BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
					BSP_LCD_FillRect(x*xInc, y*yInc, xInc, yInc);
					BSP_LCD_SetTextColor(LCD_COLOR_BLUE);
					BSP_LCD_DrawRect(x*xInc, y*yInc, xInc, yInc);
				break;
				case 2:
					BSP_LCD_SetTextColor(LCD_COLOR_BLUE);
					BSP_LCD_FillRect(x*xInc, y*yInc, xInc, yInc);
					BSP_LCD_SetTextColor(LCD_COLOR_BLUE);
					BSP_LCD_DrawRect(x*xInc, y*yInc, xInc, yInc);
				break;
				}

}

/**
  * @brief  Calculates all the rays end points and lengths
  * @note   More details about the calculations in the pdf report
  * @param  r : The starting point x coordinate of the ray
  * @param  s : The Screen used to display the game
  * @param  focalAngle : The angle of the central ray
  * @param  m : The map currently active in the game
  */
static void drawColumn(float focalAngle, Ray *r, Map *m, Screen *s)
{
	//fish eye fix
	//the following rows fixes distortions making the image quite similar to the one of a panoramic lens.
	float ca = focalAngle-r->angle;
	if(ca<0)
		ca+=2*M_PI;
	else if(ca>2*M_PI)
		ca-=2*M_PI;
	r->distance*= cos(ca); //we tune the distance to avoid the distortion

	float lineH = (m->blockSize*s->height) / r->distance;
	float lineOffset = (s->height/2)-lineH / 2 ;
	if(lineH > s->height)
		lineH = s->height;

	//color selection
	if(m->map[(int)r->pos.y/m->blockSize*m->mapBlockX+(int)(r->pos.x/m->blockSize)] == 2) //hit final wall
	{
		if(r->vertical)
			BSP_LCD_SetTextColor(LCD_COLOR_BLUE);
		else
			BSP_LCD_SetTextColor(LCD_COLOR_DARKBLUE);
	}
	else //hit wall
	{
		if(r->vertical)
			BSP_LCD_SetTextColor(LCD_COLOR_BROWN);
		else
			BSP_LCD_SetTextColor(LCD_COLOR_DARKRED);
	}

	//the last rectangle saldy given the terrible aspect ration of the display will be a little bit tighter
	//13 pixel is the usual width
	//7 is only for the last one
	int rectLeng = r->index < FOV-1 ? 13 : 7;

	BSP_LCD_FillRect(r->index*13, lineOffset, rectLeng, lineH);

	BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
	BSP_LCD_DrawRect(r->index*13, lineOffset, rectLeng, lineH);
}

/**
  * @brief  Draws a single ray on the map
  * @param  focalX : The starting point x coordinate of the ray
  * @param  focalY : The ending point y coordinate of the ray
  * @param  r : The actual ray
  */
static void drawRayMap(float focalX, float focalY, Ray *r)
{
	BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
	BSP_LCD_DrawLine(focalX / MAP_SCALE, focalY / MAP_SCALE, r->pos.x / MAP_SCALE, r->pos.y / MAP_SCALE);
}

/**
  * @brief  Calculates all the rays end points and lengths
  * @note   More details about the calculations in the pdf report
  * @param  focalX : The starting point x coordinate of the ray
  * @param  focalY : The ending point y coordinate of the ray
  * @param  focalAngle : The angle of the central ray
  * @param  m : The map currently active in the game
  */
void castRays(float focalX, float focalY, float focalAngle, Map *m)
{
	//the hearth of the rendering "engine"
	int mapX, mapY, mapIndex, dof;
	float rayX, rayY, rayAngle, xOffset, yOffset, finalDistance;

	rayAngle = focalAngle - DEGREE_RADIAN*31;
	if(rayAngle < 0)
		rayAngle += 2*M_PI;
	else if(rayAngle > 2*M_PI)
		rayAngle -= 2*M_PI;

	rayIndex = 0;

	for(int r = 0; r < FOV; r++) //60 degress would be a decent FOV but we need 62 degress due to the terrible aspect ratio of the display
	{
		//check h lines
		dof = 0;
		float disH = 100000000, hx=focalX, hy=focalY;
		float aTan = -1/tan(rayAngle);
		if(rayAngle > M_PI) //looking up
		{
			rayY = (((int)focalY / m->blockSize)*m->blockSize)-0.0001;
			rayX = (focalY-rayY)*aTan+focalX;
			yOffset = -m->blockSize;
			xOffset = -yOffset*aTan;
		}
		else if( rayAngle < M_PI) //looking down
		{
			rayY = (((int)focalY / m->blockSize)*m->blockSize)+m->blockSize;
			rayX = (focalY-rayY)*aTan+focalX;
			yOffset = m->blockSize;
			xOffset = -yOffset*aTan;
		}
		else if(rayAngle == 0 || rayAngle==M_PI)
		{
			rayX = focalX;
			rayY = focalY;
			dof = m->mapBlockY;
		}
		while(dof<m->mapBlockY)
		{
			mapX = (int)(rayX) / m->blockSize;
			mapY = (int)(rayY) / m->blockSize;
			mapIndex = mapY*m->mapBlockX+mapX;
			if(mapIndex >= 0 && mapIndex < m->mapBlockX * m->mapBlockY && m->map[mapIndex] != 0) //hit wall
			{
				hx = rayX;
				hy = rayY;
				disH = distance(focalX, focalY, hx, hy);
				dof = m->mapBlockY;
			}
			else
			{
				rayX += xOffset;
				rayY += yOffset;
				dof++;
			}
		}


		//check v lines
		dof = 0;
		float disV = 100000000, vx=focalX, vy=focalY;
		float nTan = -tan(rayAngle);
		if(rayAngle>P2 && rayAngle<P3) //looking left
		{
			rayX = (((int)focalX / m->blockSize)*m->blockSize)-0.0001;
			rayY = (focalX-rayX)*nTan+focalY;
			xOffset = -m->blockSize;
			yOffset = -xOffset*nTan;
		}
		else if(rayAngle<P2 || rayAngle>P3) //looking right
		{
			rayX = (((int)focalX  / m->blockSize)*m->blockSize)+m->blockSize;
			rayY = (focalX-rayX)*nTan+focalY;
			xOffset = m->blockSize;
			yOffset = -xOffset*nTan;
		}
		else if(rayAngle == 0 || rayAngle==M_PI) //up or down
		{
			rayX = focalX;
			rayY = focalY;
			dof = m->mapBlockX;
		}
		while(dof<m->mapBlockX)
		{
			mapX = (int)(rayX) / m->blockSize;
			mapY = (int)(rayY) / m->blockSize;
			mapIndex = mapY*m->mapBlockX+mapX;
			if(mapIndex >= 0 && mapIndex < m->mapBlockX * m->mapBlockY && m->map[mapIndex] != 0) //hit wall
			{
				vx = rayX;
				vy = rayY;
				disV = distance(focalX, focalY, vx, vy);
				dof = m->mapBlockX;
			}
			else
			{
				rayX+=xOffset;
				rayY+=yOffset;
				dof++;
			}
		}

		bool isVertical = false;

		if(disV < disH)
		{
			rayX = vx;
			rayY = vy;
			finalDistance = disV;
			isVertical = true;
		}
		else
		{
			rayX = hx;
			rayY = hy;
			finalDistance = disH;
		}


		Ray ray;
		ray.pos.x = rayX;
		ray.pos.y = rayY;
		ray.angle = rayAngle;
		ray.index = r;
		ray.distance = finalDistance;
		ray.vertical = isVertical;

		rays[rayIndex++] = ray;

		rayAngle += DEGREE_RADIAN;
		if(rayAngle < 0)
			rayAngle += 2*M_PI;
		else if(rayAngle > 2*M_PI)
			rayAngle -= 2*M_PI;
	}
}

/**
  * @brief  It draws on the top half of the screen a color used to represent the ceiling or sky of the 3D scene,
  * while it draws in the bottom half another color used to represent the terrain
  * @param  s : The Screen used to display the game
  */
void drawBackground(Screen *s)
{
	BSP_LCD_SetTextColor(LCD_COLOR_DARKYELLOW);
	BSP_LCD_FillRect(0, 0, s->width, s->height/2);

	BSP_LCD_SetTextColor(LCD_COLOR_DARKGRAY);
	BSP_LCD_FillRect(0, s->height/2, s->width, s->height/2);
}

/**
  * @brief  It renders the 3D scene with the pre-casted rays
  * @param  s : The Screen used to display the game
  * @param  m : The map currently active in the game
  * @param  focalAngle : the angle of the central ray
  */
void drawRays(Map *m, Screen *s, float focalAngle)
{
	for(int i = 0; i<FOV; i++)
	{
		Ray r = rays[i];
		drawColumn(focalAngle, &r, m, s);
	}
}

/**
  * @brief  It draws the pre-casted rays on the 2D map
  * @param  focalX : the x coordinate of the starting point used to draw all the rays
  * @param  focalY : the y coordinate of the starting point used to draw all the rays
  */
void drawMapRays(float focalX, float focalY)
{
	for(int i = 0; i<FOV; i++)
	{
		Ray r = rays[i];
		drawRayMap(focalX, focalY, &r);
	}
}

/**
  * @brief  It draws the control that can be used to move the player in the game
  * @param  s : The Screen used to display the game
  * @param  m : The map currently active in the game
  * @param  scale : the current scale compared to the size of a rectangle of the map
  */
void drawControls(Screen *s, Map *m, int scale)
{
	int mapBlockX = m->mapBlockX / scale;
	int mapBlockY = m->mapBlockY / scale;
	int blockSize = m->blockSize * scale;

	BSP_LCD_SetBackColor(LCD_COLOR_ORANGE);
	BSP_LCD_SetTextColor(LCD_COLOR_ORANGE);

	BSP_LCD_FillRect(0, (s->height/mapBlockY)*(mapBlockY-2), blockSize-1, blockSize-1);
	BSP_LCD_FillRect(0, (s->height/mapBlockY)*(mapBlockY-1), blockSize-1, blockSize-1);
	BSP_LCD_FillRect((s->width/mapBlockX)*(mapBlockX-3), (s->height/mapBlockY)*(mapBlockY-1), blockSize-1, blockSize-1);
	BSP_LCD_FillRect((s->width/mapBlockX)*(mapBlockX-1), (s->height/mapBlockY)*(mapBlockY-1), blockSize-1, blockSize-1);


	BSP_LCD_SetTextColor(LCD_COLOR_BLACK);


	BSP_LCD_DisplayStringAt(0, (s->height/mapBlockY)*(mapBlockY-2), (uint8_t*)" /\\", LEFT_MODE);
	BSP_LCD_DrawRect(0, (s->height/mapBlockY)*(mapBlockY-2), blockSize-1, blockSize-1);

	BSP_LCD_DisplayStringAt(0, (s->height/mapBlockY)*(mapBlockY-1), (uint8_t*)" \\/", LEFT_MODE);
	BSP_LCD_DrawRect(0, (s->height/mapBlockY)*(mapBlockY-1), blockSize-1, blockSize-1);

	BSP_LCD_DisplayStringAt((s->width/mapBlockX)*(mapBlockX-3), (s->height/mapBlockY)*(mapBlockY-1), (uint8_t*)"  <\0", LEFT_MODE);
	BSP_LCD_DrawRect((s->width/mapBlockX)*(mapBlockX-3), (s->height/mapBlockY)*(mapBlockY-1), blockSize-1, blockSize-1);

	BSP_LCD_DisplayStringAt((s->width/mapBlockX)*(mapBlockX-1), (s->height/mapBlockY)*(mapBlockY-1), (uint8_t*)"  >\0", LEFT_MODE);
	BSP_LCD_DrawRect((s->width/mapBlockX)*(mapBlockX-1), (s->height/mapBlockY)*(mapBlockY-1), blockSize-1, blockSize-1);

}


