/*
 * screen.h
 *
 *  Created on: 5 dic 2022
 *      Author: fabio
 */

#ifndef INC_RENDER_SCREEN_H_
#define INC_RENDER_SCREEN_H_


#include "FreeRTOS.h"
#include "semphr.h"

typedef struct {
	uint32_t addr[2];
	uint32_t width;
	uint32_t height;
	uint32_t front;
	SemaphoreHandle_t *lcd_mut;
} Screen;

Screen* ct_screen_init();
void ct_screen_flip_buffers(Screen *screen);
uint32_t* ct_screen_backbuffer_ptr(Screen *screen);

extern Screen *screen;

#endif /* INC_RENDER_SCREEN_H_ */
