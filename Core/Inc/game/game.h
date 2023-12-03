/*
 * game.h
 *
 *  Created on: 5 dic 2022
 *      Author: fabio
 */

#ifndef INC_GAME_GAME_H_
#define INC_GAME_GAME_H_

#include "render/render.h"
#include "render/screen.h"
#include "FreeRTOS.h"
#include "semphr.h"

typedef struct {
	vec2 pos;
	vec2 initial_pos;
	float dx;
	float dy;
	float angle;
	SemaphoreHandle_t *player_pos_mut;
} Player;

void showStartScreen(Screen *s, bool show);
void showPauseScreen(Screen *s, bool show);
void playerMovementTouch(Player *p, Map *m, Screen *s, int scale);
void playerMovementKeyboard(Player *p, Map *m, char command);
void drawMapPlayer(Player *p);
void gameLogic(Player *p, Map *m, Screen *s);

extern TaskHandle_t flashing_text_task_handle;

#endif /*INC_GAME_GAME_H_*/
