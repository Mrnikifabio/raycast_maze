/*************************************************************************//**
 *
 *    \file	    	main_user.c
 *    \date			14.01.2022 / 09.12.2021
 *    \version  	0.1.0
 *
 *    \author 		Crugnola Fabio SUPSI DTI I3A
 *
 ******************************************************************************
 *
 *					HW REQUIREMENT TO USE THE DISPLAY (B-LCD40-DSI1):
 *					- TIM3: configured (used by display driverI
 *					- FMC: configured to use the external RAM (MTC48LC4M32B2)
 *					- I2C4: configured to communicate with the touch screen
 *						    interface.
 *					- DMA2D: configured to transfer data to the display.
 *					- DSIHOST: (Display Serial Interface HOST) configured to
 *					 		   communicate with the display.
 *					- LTDC: (LCT-TFT Display Controller) configured to
 *							communicate with the display.
 *
 *
 *	 				Compiler: arm-gcc 9.9.0.201906111633

 *
 ******************************************************************************/

#include "main_user.h"
#include "usart.h"
#include "semphr.h"
#include "render/screen.h"
#include "render/render.h"
#include "game/game.h"
#include "stm32f769i_discovery_lcd.h"
#include "tim.h"

#include <stdio.h>
#include <string.h>

/* Private define ------------------------------------------------------------*/
/* Private data types definition ---------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
TaskHandle_t uart_rx_task_handler;
TaskHandle_t button_task_handler;
/* Private variables ---------------------------------------------------------*/
static TaskHandle_t main_task_handler;	//main task handle
static SemaphoreHandle_t player_pos_mut;  //mutex used by various tasks to claim the player position object
static Player p; //the player object represents the user in the game
static Map map; //the map object represents the current map that is used to render the 3D world


static bool showMap;
static bool showFPSCounter;
static bool firstLaunch;
static bool pause;
static bool showText; //used to animate the text in the welcome and pause screen
static int frameCounter = 0;
static int frameCounterToShow = 0; //current fps value to actually print on the screen


/* Private function prototypes -----------------------------------------------*/
static void main_task( void *pvParameters );
static void uart_task( void *pvParameters );
static void button_task(void *pvParameters);
static void cmd_parser_execute(char *cmd);
static void navigation_mode();
static void show_menu();

/* Functions definition ------------------------------------------------------*/
/**
  * @brief Create the FreeRTOS objects and tasks. Configures initial player position and direction. Configures initial game settings e map size.
  * @return true if the tasks are created, false otherwise.
  */
void freeRTOS_user_init(void)
{
	player_pos_mut = xSemaphoreCreateMutex();
	p.player_pos_mut = &player_pos_mut;

	//number of horizontal blocks in the map
	map.mapBlockX = 20;
	//number of vertical blocks in the map
	map.mapBlockY = 12;

	//size in pixel of a block
	map.blockSize = 40;

	showMap = false;

	//load the first map
	changeMap(&map);

	firstLaunch = true;
	showFPSCounter = true;
	pause = false;
	showText = true;

	p.initial_pos.x = 95; p.initial_pos.y = 320;
	p.pos.x=95; p.pos.y=320;

	//set initial direction of the player
	p.dx = cos(p.angle)*5;
	p.dy = sin(p.angle)*5;

	xTaskCreate( main_task,		//Task function
				"main_task",					//Task function comment
				256,							//Task stack dimension (1kB)
				NULL,							//Task parameter
				1,								//Task priority
				&main_task_handler );			//Task handle

	xTaskCreate(button_task, "button_task", configMINIMAL_STACK_SIZE, NULL, 1, &button_task_handler);
	xTaskCreate(uart_task, "uart_task", configMINIMAL_STACK_SIZE, NULL, 1, &uart_rx_task_handler);
}

/**
  * @brief  Main loop task: every cycle renders a frame and handles the game logic.
  * @param pvParameters : void* parameters that might be needed by the task
  */
static void main_task( void *pvParameters )
{
	char fps[10];
	HAL_TIM_Base_Start_IT(&htim2);

	while(1){
		ct_screen_flip_buffers(screen);

		if(firstLaunch)
			showStartScreen(screen, showText);
		else if(!pause)
		{
			playerMovementTouch(&p, &map, screen, 2);
			drawBackground(screen);
			castRays(p.pos.x, p.pos.y, p.angle, &map);
			drawRays(&map, screen, p.angle);

			if(showMap)
			{
				drawMap(&map, screen);
				drawMapRays(p.pos.x, p.pos.y);
				drawMapPlayer(&p);
			}

			drawControls(screen, &map, 2);
			gameLogic(&p, &map, screen);

			//FPS COUNTER
			frameCounter++;
			if(showFPSCounter)
			{
				sprintf(fps, "%d FPS", 1000/frameCounterToShow);
				BSP_LCD_DisplayStringAt(0, 0, (uint8_t*)fps, RIGHT_MODE);
			}
		}
		else
			showPauseScreen(screen, showText);
	}
}

/**
  * @brief Callback called by Timer2 ISR. Timer2 timeout expires every second.
  * @note The function resets the fps counter used in the main loop.
  * @note It also negate the boolean value used to animate text in the welcome and pause screen
  */
void secondElapsed()
{
	showText = !showText;
	frameCounterToShow = frameCounter;
	frameCounter = 0;
}

/**
  * @brief  The button task waits for a notification coming from the GPIO ISR. When the notification arrives it means that the USER button has been pressed
  * @note   When the main loop is stuck in the welcome screen, if the button is pressed it starts the game, otherwise it pauses or resume the game.
  * @param  pvParameters : void* parameters that might be used by the task
  */
static void button_task(void *pvParameters)
{
	uint32_t byte;
	while(1)
	{
		xTaskNotifyWait(0, 0, &byte, portMAX_DELAY); //wait for button to be pressed

		if(firstLaunch)
			firstLaunch = false;
		else
			pause = !pause;
	}
}

/**
  * @brief  Checks if target device is ready for communication.
  * @note   This function is used with Memory devices
  * @param  i2c_handler : I2C handler
  * @param  DevAddress: Target device address
  * @param  Trials: Number of trials
  * @retval HAL status
  */
static void uart_task(void *pvParameters)
{
	char cmd_buffer[20];
	int i = 0;
	uint32_t byte;

	while(1)
	{
		HAL_UART_Receive_IT(&huart1, (uint8_t *)&huart1.Instance->RDR, sizeof(char));
		xTaskNotifyWait(0, 0, &byte, portMAX_DELAY);
		cmd_buffer[i++] = (char)byte;
		if(byte != '\r' && i <= 17)
			continue;
		cmd_buffer[i++] = '\n';
		cmd_buffer[i++] = '\0';
		cmd_parser_execute(cmd_buffer);
		memset(cmd_buffer, 0, sizeof(cmd_buffer));
		i = 0;
	}
}

/**
  * @brief  handles the various options of the menu based on the string it receives as a parameter. The function works only if the player is already after the welcome screen
  * @param  cmd : *char
  */
static void cmd_parser_execute(char *cmd)
{
	if(firstLaunch)
		firstLaunch = false;
	else
	{
		if(cmd[0] == 'p')
			pause = !pause;
		else if(!pause)
		{
			switch(cmd[0])
			{
			case 'n':
				navigation_mode();
				break;
			case 'b':
				showMap = !showMap;
				break;
			case 'f':
				showFPSCounter = !showFPSCounter;
			default:
				show_menu();
				break;
			}
		}
	}
}

/**
  * @brief Sends to USART1 a string containing the various options available from the menu.
  */
static void show_menu()
{
	char menu[] = "m. Show menu\r\nn. Control player\r\nb. Show Map\r\nf. Show FPS Counter\r\np. Play / Pause\r\n";
	HAL_UART_Transmit(&huart1, (unsigned char*)menu, strlen(menu)*sizeof(char), -1);
}

/**
  * @brief  Starts an infinite loop where it listens for characters coming from USART1.
  * @note   When it receives w, a, s or d as characters it makes a call to playerMovementKeyboard so that the player position can be changed as a consequence.
  * @note 	When n is received the loop ends.
  */
static void navigation_mode()
{
	uint32_t byte;
	while(1)
	{
		HAL_UART_Receive_IT(&huart1, (uint8_t *)&huart1.Instance->RDR, sizeof(char));
		xTaskNotifyWait(0, 0, &byte, portMAX_DELAY);
		if((char)byte == 'n')
			break;
		else
			playerMovementKeyboard(&p, &map, (char)byte);
	}
}
