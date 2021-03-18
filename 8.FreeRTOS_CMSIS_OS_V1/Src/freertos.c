/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
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
/*�����ʱ��*/
osTimerId led_blink_timer_id ;
void led_blink_callback(void const * argument);
osTimerDef(led_blink_timer_handle, led_blink_callback);

/*������*/
osMutexId osMutex;
osMutexDef(osMutex);

/*��Ϣ����*/
#define QUEUE_SIZE 10
osMessageQId osQueue ;
osMessageQDef(osqueue, QUEUE_SIZE, uint16_t);

/*�ź���*/
osSemaphoreId  led_sem_id;
osSemaphoreDef(sync_sem);

/*LED�߳�*/
osThreadId LedTaskHandle;
uint32_t LedTaskBuffer[ 512 ];
osStaticThreadDef_t LedTaskControlBlock;

/*��ӡ�߳�*/
osThreadId PrintfTaskHandle;
uint32_t PrintfTaskBuffer[ 512 ];
osStaticThreadDef_t PrintfTaskControlBlock;


/*�������߳�*/
osThreadId ProTaskHandle;
uint32_t ProTaskBuffer[ 512 ];
osStaticThreadDef_t ProTaskControlBlock;

/*�������߳�*/
osThreadId ConTaskHandle;
uint32_t ConTaskBuffer[ 512 ];
osStaticThreadDef_t ConTaskControlBlock;
/* USER CODE END Variables */
osThreadId defaultTaskHandle;
uint32_t defaultTaskBuffer[ 512 ];
osStaticThreadDef_t defaultTaskControlBlock;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void StartLedTask(void const * argument);
void StartPrintfTask(void const * argument);

/*������*/
void StartProTask(void const * argument);
/*������*/
void StartConTask(void const * argument);
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* GetTimerTaskMemory prototype (linked to static allocation support) */
void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize );

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = &xIdleStack[0];
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
  /* place for user code */
}
/* USER CODE END GET_IDLE_TASK_MEMORY */

/* USER CODE BEGIN GET_TIMER_TASK_MEMORY */
static StaticTask_t xTimerTaskTCBBuffer;
static StackType_t xTimerStack[configTIMER_TASK_STACK_DEPTH];

void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize )
{
  *ppxTimerTaskTCBBuffer = &xTimerTaskTCBBuffer;
  *ppxTimerTaskStackBuffer = &xTimerStack[0];
  *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
  /* place for user code */
}
/* USER CODE END GET_TIMER_TASK_MEMORY */

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
	/*����������*/
	osMutex = osMutexCreate(osMutex(osMutex));
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
	/*����һ��LED��˸���ź���*/
	led_sem_id = osSemaphoreCreate(osSemaphore(sync_sem), 1);
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
	/*����һ�������ʱ��===>osTimerPeriodicѭ������*/
	led_blink_timer_id = osTimerCreate(osTimer(led_blink_timer_handle),osTimerPeriodic,NULL);
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
	/*������Ϣ����*/
	osQueue = osMessageCreate (osMessageQ(osqueue), NULL);
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadStaticDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 512, defaultTaskBuffer, &defaultTaskControlBlock);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument)
{
  /* USER CODE BEGIN StartDefaultTask */
	/*1������������LED�߳�*/
	osThreadStaticDef(ledTask, StartLedTask, osPriorityNormal, 0, 512, LedTaskBuffer, &LedTaskControlBlock);
  LedTaskHandle = osThreadCreate(osThread(ledTask), NULL);
	/*2��������������ӡ�߳�*/
	osThreadStaticDef(PrintfTask, StartPrintfTask, osPriorityRealtime, 0, 512, PrintfTaskBuffer, &PrintfTaskControlBlock);
  PrintfTaskHandle = osThreadCreate(osThread(PrintfTask), NULL);
	/*3�������������������߳�*/
	osThreadStaticDef(ProTask, StartProTask, osPriorityAboveNormal, 0, 512, ProTaskBuffer, &ProTaskControlBlock);
  ProTaskHandle = osThreadCreate(osThread(ProTask), NULL);
	/*4�������������������߳�*/
	osThreadStaticDef(ConTask, StartConTask, osPriorityAboveNormal, 0, 512, ConTaskBuffer, &ConTaskControlBlock);
  ConTaskHandle = osThreadCreate(osThread(ConTask), NULL);
  /* Infinite loop */
  for(;;)
  {
		/*�ͷ�һ���ź���*/
		osSemaphoreRelease(led_sem_id);
    osDelay(3000);
  }
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
/*�����ʱ���ص�����*/
void led_blink_callback(void const * argument)
{
	HAL_GPIO_TogglePin(GS_LED_GPIO_Port,GS_LED_Pin);
}

/*LED�߳�*/
void StartLedTask(void const * argument)
{
	/*���������ʱ����ÿ200ms��һ�η�ת*/
	osTimerStart(led_blink_timer_id,200);
	for(;;)
	{
		osSemaphoreWait(led_sem_id,osWaitForever);
		HAL_GPIO_TogglePin(ALS_LED_GPIO_Port,ALS_LED_Pin);
	}
}

/*��ӡ�߳�*/
void StartPrintfTask(void const * argument)
{
	uint32_t Count = 0 ;
	for(;;)
	{
		/*�ͷ�һ��������*/
		osMutexRelease(osMutex);
		Count++ ;
		printf("Count=%d\n",Count);
		/*�ȴ�һ��������*/
		osMutexWait(osMutex,osWaitForever);
		osDelay(1000);
	}
}

/*������*/
void StartProTask(void const * argument)
{
	static uint8_t  i = 0;
	uint32_t Probuffer[10] = {1,2,3,4,5,6,7,8,9,10};
	for(;;)
	{
		if(i > 9)
			i = 0 ;
		osMessagePut(osQueue,Probuffer[i],0);
		osDelay(1000);
		i++;
	}
}

/*������*/
void StartConTask(void const * argument)
{
	osEvent event ;
	for(;;)
	{
		event = osMessageGet(osQueue,osWaitForever);
		printf("��ȡ����event.value:%d\n",event.value.v);
		HAL_GPIO_TogglePin(COMPASS_LED_GPIO_Port,COMPASS_LED_Pin);
	}
}
/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
