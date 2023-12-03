#define SCREEN_W 	800
#define SCREEN_H	480

#include "render/screen.h"
#include "stm32f769i_discovery_lcd.h"
#include <stdlib.h>

//instance of the screen that gets initialized and then returned by ct_screen_init()
Screen *screen;

/**
  * @param  s : The Screen used to display the game
  * @return An integer used to identify the backbuffer display
  */
uint32_t ct_screen_backbuffer_id(Screen *screen) {
	return 1 - screen->front;
}

/**
  * @brief Configures the display so that it can be used
  * @return A Screen pointer referencing the screen that has just been initialized
  */
Screen* ct_screen_init() {
	BSP_LCD_Init();
	screen = (Screen*) malloc(sizeof(Screen));
	screen->width = BSP_LCD_GetXSize();
	screen->height = BSP_LCD_GetYSize();
	screen->addr[0] = LCD_FB_START_ADDRESS;
	screen->addr[1] = LCD_FB_START_ADDRESS + screen->width * screen->height * 4;
	screen->front = 1;
	BSP_LCD_LayerDefaultInit(0, screen->addr[0]);
	BSP_LCD_LayerDefaultInit(1, screen->addr[1]);
	BSP_LCD_SetLayerVisible(0, DISABLE);
	BSP_LCD_SetLayerVisible(1, ENABLE);
	BSP_LCD_SelectLayer(0);
	return screen;
}

/**
  * @param  s : The Screen used to display the game
  * @return the pointer to the back buffer of the display
  */
uint32_t* ct_screen_backbuffer_ptr(Screen *screen) {
	return (uint32_t*)(screen->addr[ct_screen_backbuffer_id(screen)]);
}

/**
  * @brief  It draws the pause screen
  * @param  s : The Screen used to display the game
  */
void ct_screen_flip_buffers(Screen *screen) {
  // wait for VSYNC
	while (!(LTDC->CDSR & LTDC_CDSR_VSYNCS));
	BSP_LCD_SetLayerVisible(screen->front, DISABLE);
	screen->front ^= 1;
	BSP_LCD_SetLayerVisible(screen->front, ENABLE);
	BSP_LCD_SelectLayer(ct_screen_backbuffer_id(screen));
}

