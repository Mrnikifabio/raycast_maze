#include "game/game.h"
#include "render/map.h"
#include "stm32f769i_discovery_ts.h"
#include "stm32f769i_discovery_lcd.h"
#include "task.h"
#include "timers.h"
#include <math.h>
#include <stdio.h>

static inline void rotateCW(Player *p);
static inline void rotateCCW(Player *p);
static inline void goForward(Player *p, Map *m);
static inline void goBackward(Player *p, Map *m);
static inline void rotateCW(Player *p);
static inline void rotateCCW(Player *p);
static void playerMovementTouchControls(uint16_t touchX, uint16_t touchY, Player *p, Map *m, Screen *s, int scale);
static void timer_callback();

/**
  * @brief  Moves the player forward
  * @param  p : The Player that needs to be moved
  * @param  m : The Map on which the player stays
  */
static inline void goForward(Player *p, Map *m)
{
	int offsetX = p->dx > 0 ? 8 : -8;
	int offsetY = p->dy > 0 ? 8 : -8;
	float futureX = (p->pos.x + p->dx + offsetX);
	float futureY = (p->pos.y + p->dy + offsetY);
	if(m->map[(int)(futureY/m->blockSize)*m->mapBlockX+(int)(futureX/m->blockSize)] == 1) //hit wall
		return;
	p->pos.x += p->dx;
	p->pos.y += p->dy;
}

/**
  * @brief  Moves the player backward
  * @param  p : The Player that needs to be moved
  * @param  m : The Map on which the player stays
  */
static inline void goBackward(Player *p, Map *m)
{
	int offsetX = p->dx > 0 ? 8 : -8;
	int offsetY = p->dy > 0 ? 8 : -8;
	float futureX = (p->pos.x - p->dx + offsetX);
	float futureY = (p->pos.y - p->dy + offsetY);
	if(m->map[(int)futureY/m->blockSize*m->mapBlockX+(int)(futureX/m->blockSize)] == 1) //hit wall
		return;
	p->pos.x -= p->dx;
	p->pos.y -= p->dy;
}

/**
  * @brief  Increments the player direction clockwise
  * @param  p : The Player that needs to be rotated
  * @param  m : The Map on which the player stays
  */
static inline void rotateCW(Player *p)
{
	p->angle+=0.1;
	if(p->angle > 2*M_PI)
		p->angle -= 2*M_PI;
	p->dx = cos(p->angle)*5;
	p->dy = sin(p->angle)*5;
}

/**
  * @brief  Increments the player direction counterclockwise
  * @param  p : The Player that needs to be rotated
  * @param  m : The Map on which the player stays
  */
static inline void rotateCCW(Player *p)
{
	p->angle-=0.1;
	if(p->angle < 0)
		p->angle += 2*M_PI;
	p->dx = cos(p->angle)*5;
	p->dy = sin(p->angle)*5;
}

/**
  * @brief moves the player based on the touched touch screen panel area as a consequence
  * @param  p : The Player that needs to be rotated
  * @param  m : The Map on which the player stays
  * @param  s : The Screen used to display the game and detect touches
  * @param  scale : the current scale compared to the size of a rectangle of the map of the touchable area
  * @param  touchX : the x coordinate of the touched point on the panel
  * @param  touchY : the y coordinate of the touched point on the panel
  */
static void playerMovementTouchControls(uint16_t touchX, uint16_t touchY, Player *p, Map *m, Screen *s, int scale)
{
	int mapBlockX = m->mapBlockX / scale;
	int mapBlockY = m->mapBlockY / scale;

	if(touchX > (s->width / mapBlockX) * (mapBlockX-3) && touchX < (s->width / mapBlockX) * (mapBlockX-2)  && touchY > (s->height / mapBlockY) * (mapBlockY-1))
		rotateCCW(p);
	else if(touchX > (s->width / mapBlockX) * (mapBlockX-1) && touchY > (s->height / mapBlockY) * (mapBlockY-1))
		rotateCW(p);
	else if(touchX < (s->width / mapBlockX) && touchY > (s->height / mapBlockY) * (mapBlockY-2) && touchY < (s->height / mapBlockY) * (mapBlockY-1))
		goForward(p, m);
	else if(touchX < (s->width / mapBlockX) && touchY > (s->height / mapBlockY) * (mapBlockY-1))
		goBackward(p, m);
}

/**
  * @brief  detects if the defined area of the touch screen panel has been touched and pass the coordinates to the playerMovementTouchControls function
  * @param  p : The Player that needs to be rotated
  * @param  m : The Map on which the player stays
  * @param  s : The Screen used to display the game and detect touches
  * @param  scale : the current scale compared to the size of a rectangle of the map of the touchable area
  */
void playerMovementTouch(Player *p, Map *m, Screen *s, int scale)
{
	TS_StateTypeDef TS_State;
	BSP_TS_GetState(&TS_State);

	if(TS_State.touchDetected)
	{
		BaseType_t ret;
		ret = xSemaphoreTake(*p->player_pos_mut, portMAX_DELAY);
		if(ret == pdTRUE)
		{
			playerMovementTouchControls(TS_State.touchX[0],TS_State.touchY[0], p, m, s, scale);
			if(TS_State.touchDetected == 2)
				playerMovementTouchControls(TS_State.touchX[1],TS_State.touchY[1], p, m, s, scale);
			xSemaphoreGive(*p->player_pos_mut);
		}
	}
}

/**
  * @brief  Moves the player based on the character received
  * @note w(moves forward), a(moves backward), s(rotates to the left), d (rotates to the right)
  * @param  p : The Player that needs to be rotated
  * @param  m : The Map on which the player stays
  * @param  s : The Screen used to display the game and detect touches
  * @param  c : The character that defines the movement to make
  */
void playerMovementKeyboard(Player *p, Map *m, char command)
{
	BaseType_t ret;
	ret = xSemaphoreTake(*p->player_pos_mut, portMAX_DELAY);
	if(ret == pdTRUE)
	{
		switch(command)
		{
		case 'w':
			goForward(p, m);
			break;
		case 'a':
			rotateCCW(p);
			break;
		case 's':
			goBackward(p, m);
			break;
		case 'd':
			rotateCW(p);
			break;
		}
		xSemaphoreGive(*p->player_pos_mut);
	}
}

/**
  * @brief  Draws the player position on the map with a black dot
  * @param  p : The Player that needs to be drawn
  */
void drawMapPlayer(Player *p)
{
	int x = round(p->pos.x) / MAP_SCALE;
	int y = round(p->pos.y) / MAP_SCALE;
	int destX = round(p->pos.x +p->dx*10)  / MAP_SCALE;
	int destY = round(p->pos.y +p->dy*10) / MAP_SCALE;

	BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
	BSP_LCD_DrawPixel(x, y, LCD_COLOR_BLACK);
	BSP_LCD_DrawLine(x,y,destX,destY);
}


static TimerHandle_t timer;
static int count = 3000;

/**
  * @brief  Handles what must happen when the player reaches the maze exit as explained in the report pdf document
  * @param  p : The Player that needs to be checked
  * @param  m : The Map on which the player stays
  * @param  s : The Screen used to display the game
  */
void gameLogic(Player *p, Map *m, Screen *s)
{
	if(m->map[(int)p->pos.y/m->blockSize*m->mapBlockX+(int)(p->pos.x/m->blockSize)] == 2 || count < 3000) //hit wall
	{
		BSP_LCD_Clear(LCD_COLOR_BLACK);
		BSP_LCD_DisplayStringAt(0, s->height/2, (uint8_t*)"WINNER! You've reached the exit, next map in:", CENTER_MODE);
		char count_s[12];
		sprintf(count_s, "%d seconds", count/1000);
		BSP_LCD_DisplayStringAt(0, (s->height/2) + 20, (uint8_t*)count_s, CENTER_MODE);

		BaseType_t ret;
		ret = xSemaphoreTake(*p->player_pos_mut, portMAX_DELAY);
		if(ret == pdTRUE)
		{
			p->pos.x = p->initial_pos.x;
			p->pos.y = p->initial_pos.y;
			xSemaphoreGive(*p->player_pos_mut);
		}

		if(count <= 0)
		{
			xTimerStop(timer, portMAX_DELAY);
			count = 3000;
			changeMap(m);
		}
		else if(count == 3000)
		{
			timer = xTimerCreate("tim_win", pdMS_TO_TICKS(1), pdTRUE, NULL, timer_callback);
			xTimerReset(timer, portMAX_DELAY);
			xTimerStart(timer, portMAX_DELAY);
		}
	}
}

/**
  * @brief  Callback called by the software timer defined at line 181. It is used to show the screen of congratulations when the player reaches the maze exit
  */
void timer_callback()
{
	count--;
}

/**
  * @brief  It draws the pause screen
  * @param  s : The Screen used to display the game
  * @param  show : when set to true it displays the string "PAUSE" on the display
  */
void showPauseScreen(Screen *s, bool show)
{
	BSP_LCD_Clear(LCD_COLOR_ORANGE);
	BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
	if(show)
		BSP_LCD_DisplayStringAt(0, s->height/2, (uint8_t*)"PAUSE", CENTER_MODE);
}

/**
  * @brief  It draws the welcome screen
  * @param  s : The Screen used to display the game
  * @param  show : when set to true it displays the string "PRESS THE USER BUTTON TO PLAY" on the display
  */
void showStartScreen(Screen *s, bool show)
{
	BSP_LCD_Clear(LCD_COLOR_ORANGE);
	BSP_LCD_DisplayStringAt(0, s->height/2, (uint8_t*)"RAYCAST MAZE", CENTER_MODE);

	if(show)
	{
		BSP_LCD_DisplayStringAt(0, (s->height)/2 + 30, (uint8_t*)"PRESS THE USER BUTTON TO PLAY", CENTER_MODE);
	}

	BSP_LCD_DisplayStringAt(0, (s->height) - 25, (uint8_t*)"Mrnikifabio 2022, SUPSI DTI-ISEA", CENTER_MODE);
}
