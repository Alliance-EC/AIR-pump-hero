/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * File Name          : freertos.c
 * Description        : Code for freertos applications
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2023 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for Test */
osThreadId_t TestHandle;
const osThreadAttr_t Test_attributes = {
  .name = "Test",
  .stack_size = 1024 * 4,
  .priority = (osPriority_t) osPriorityHigh,
};
/* Definitions for Buzzer */
osThreadId_t BuzzerHandle;
const osThreadAttr_t Buzzer_attributes = {
  .name = "Buzzer",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for robotask */
osThreadId_t robotaskHandle;
const osThreadAttr_t robotask_attributes = {
  .name = "robotask",
  .stack_size = 1024 * 4,
  .priority = (osPriority_t) osPriorityHigh,
};
/* Definitions for LEDtask */
osThreadId_t LEDtaskHandle;
const osThreadAttr_t LEDtask_attributes = {
  .name = "LEDtask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for insTask */
osThreadId_t insTaskHandle;
const osThreadAttr_t insTask_attributes = {
  .name = "insTask",
  .stack_size = 2048 * 4,
  .priority = (osPriority_t) osPriorityRealtime5,
};
/* Definitions for UITask */
osThreadId_t UITaskHandle;
const osThreadAttr_t UITask_attributes = {
  .name = "UITask",
  .stack_size = 1024 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void TestTask(void *argument);
void BuzzerTask(void *argument);
void Startrobottask(void *argument);
void StartLEDtask(void *argument);
void StartINSTASK(void *argument);
void UItask(void *argument);

extern void MX_USB_DEVICE_Init(void);
void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
    /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
    /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
    /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
    /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of Test */
  TestHandle = osThreadNew(TestTask, NULL, &Test_attributes);

  /* creation of Buzzer */
  BuzzerHandle = osThreadNew(BuzzerTask, NULL, &Buzzer_attributes);

  /* creation of robotask */
  robotaskHandle = osThreadNew(Startrobottask, NULL, &robotask_attributes);

  /* creation of LEDtask */
  LEDtaskHandle = osThreadNew(StartLEDtask, NULL, &LEDtask_attributes);

  /* creation of insTask */
  insTaskHandle = osThreadNew(StartINSTASK, NULL, &insTask_attributes);

  /* creation of UITask */
  UITaskHandle = osThreadNew(UItask, NULL, &UITask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
    /* add threads, ... */

  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
    /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
 * @brief  Function implementing the defaultTask thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* init code for USB_DEVICE */
  MX_USB_DEVICE_Init();
  /* USER CODE BEGIN StartDefaultTask */
    UNUSED(argument);
    while(1)
    {
      osThreadTerminate(NULL); // 避免空置和切换占用cpu
      osDelay(10);
    }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_TestTask */
/**
 * @brief Function implementing the Test thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_TestTask */
__weak void TestTask(void *argument)
{
  /* USER CODE BEGIN TestTask */
  UNUSED(argument);
    /* Infinite loop */
    for (;;) {
        osDelay(1);
    }
  /* USER CODE END TestTask */
}

/* USER CODE BEGIN Header_BuzzerTask */
/**
* @brief Function implementing the Buzzer thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_BuzzerTask */
__weak void BuzzerTask(void *argument)
{
  /* USER CODE BEGIN BuzzerTask */
  UNUSED(argument);
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END BuzzerTask */
}

/* USER CODE BEGIN Header_Startrobottask */
/**
* @brief Function implementing the robotask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Startrobottask */
__weak void Startrobottask(void *argument)
{
  /* USER CODE BEGIN Startrobottask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END Startrobottask */
}

/* USER CODE BEGIN Header_StartLEDtask */
/**
* @brief Function implementing the LEDtask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartLEDtask */
__weak void StartLEDtask(void *argument)
{
  /* USER CODE BEGIN StartLEDtask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartLEDtask */
}

/* USER CODE BEGIN Header_StartINSTASK */
/**
* @brief Function implementing the myTask06v thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartINSTASK */
__weak void StartINSTASK(void *argument)
{
  /* USER CODE BEGIN StartINSTASK */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartINSTASK */
}

/* USER CODE BEGIN Header_UItask */
/**
* @brief Function implementing the UITask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_UItask */
__weak void UItask(void *argument)
{
  /* USER CODE BEGIN UItask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END UItask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

