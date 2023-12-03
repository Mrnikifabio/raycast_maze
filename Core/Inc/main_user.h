/*************************************************************************//**
 *
 *    \file	    	main_user.c
 *    \date			19.11.2021 / 18.11.2021
 *    \version  	0.1.0
 *
 *    \author 		Crugnola Fabio SUPSI DTI I3A
 *
 ******************************************************************************/

#ifndef INC_MAIN_USER_H_
#define INC_MAIN_USER_H_

#include "FreeRTOS.h"
#include "task.h"

/* Public define -------------------------------------------------------------*/

/* Public variables ----------------------------------------------------------*/
extern TaskHandle_t uart_rx_task_handler;
extern TaskHandle_t button_task_handler;
/* Public function prototypes ------------------------------------------------*/
void freeRTOS_user_init(void);
void secondElapsed();

#endif /* INC_MAIN_USER_H_ */
