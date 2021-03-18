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
#define BUFFER_SIZE 1U
/*软件定时器*/
osTimerId_t led_blink_timer_id ;
/*信号量*/
osSemaphoreId_t led_sem_id;
/*互斥量*/
osMutexId_t     osMutex;
const osMutexAttr_t Mutex_attr = {
  "MyMutex",                          // human readable mutex name
  osMutexRecursive | osMutexPrioInherit,    // attr_bits
  NULL,                                     // memory for control block   
  0U                                        // size for control block
};
/*消息队列*/
#define MSGQUEUE_OBJECTS 16                     // number of Message Queue Objects
osMessageQueueId_t osQueue ;
typedef struct {                                // object data type
  uint8_t Buf[10];
} MSGQUEUE_OBJ_t;



/*LED线程*/
osThreadId_t LedTaskHandle;
const osThreadAttr_t ledTask_attributes = {
  .name = "LedTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 512 
};

/*打印线程*/
osThreadId_t PrintfTaskHandle;
const osThreadAttr_t PrintfTask_attributes = {
  .name = "PrintfTask",
  .priority = (osPriority_t) osPriorityRealtime,
  .stack_size = 512 
};

/*生产者线程*/
osThreadId_t ProTaskHandle;
const osThreadAttr_t ProTask_attributes = {
  .name = "ProTask",
  .priority = (osPriority_t) osPriorityLow,
  .stack_size = 512 
};

/*消费者线程*/
osThreadId_t ConTaskHandle;
const osThreadAttr_t ConTask_attributes = {
  .name = "ConTask",
  .priority = (osPriority_t) osPriorityLow,
  .stack_size = 512 
};

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 512 * 4
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void StartProTask(void *argument);
void StartConTask(void *argument);
void StartLedTask(void *argument);
void StartPrintfTask(void *argument);


/*软件定时器回调函数*/
void led_blink_callback(void *argument);
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);

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
	/*创建一个互斥量*/
	osMutex = osMutexNew(&Mutex_attr);
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
	/*创建一个信号量*/
	led_sem_id =  osSemaphoreNew(BUFFER_SIZE, BUFFER_SIZE, NULL);
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
	/*创建一个定时器*/
	led_blink_timer_id = osTimerNew(led_blink_callback,osTimerPeriodic,NULL,NULL);
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
	/*创建消息队列*/
	osQueue = osMessageQueueNew(MSGQUEUE_OBJECTS, sizeof(MSGQUEUE_OBJ_t), NULL);
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

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
  /* USER CODE BEGIN StartDefaultTask */
	/*创建其它线程*/
	LedTaskHandle = osThreadNew(StartLedTask, NULL, &ledTask_attributes);
	if(LedTaskHandle != NULL)
	{
		printf("创建LedTaskHandle成功!\n");
	}
	PrintfTaskHandle = osThreadNew(StartPrintfTask, NULL, &PrintfTask_attributes);
	if(PrintfTaskHandle != NULL)
	{
		printf("创建PrintfTaskHandle成功!\n");
	}
	ProTaskHandle = osThreadNew(StartProTask, NULL, &ProTask_attributes);
	if(ProTaskHandle != NULL)
	{
		printf("创建ProTaskHandle成功!\n");
	}
	else
	{
		printf("创建ProTaskHandle失败!\n");
	}
	ConTaskHandle = osThreadNew(StartConTask, NULL, &ConTask_attributes);
	if(ConTaskHandle != NULL)
	{
		printf("创建ConTaskHandle成功!\n");
	}
	else
	{
		printf("创建ConTaskHandle失败!\n");
	}
  /* Infinite loop */
  for(;;)
  {
    /*释放一个信号量*/
		osSemaphoreRelease(led_sem_id);
    osDelay(3000);
  }
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
/*软件定时器回调函数*/
void led_blink_callback(void  *argument)
{
	HAL_GPIO_TogglePin(GS_LED_GPIO_Port,GS_LED_Pin);
}

/*LED线程*/
void StartLedTask(void *argument)
{
	printf("LED线程\n");
	/*启动软件定时器，每200ms做一次翻转*/
	osTimerStart(led_blink_timer_id,200);
	for(;;)
  {
		osSemaphoreAcquire(led_sem_id,osWaitForever);
		HAL_GPIO_TogglePin(ALS_LED_GPIO_Port,ALS_LED_Pin);
  }
}

/*打印线程*/
void StartPrintfTask(void *argument)
{
	uint32_t Count = 0 ;
	printf("打印线程\n");
	for(;;)
  {
    /*释放一个互斥量*/
		osMutexRelease(osMutex);
		Count++ ;
		printf("Count=%d\n",Count);
		/*等待一个互斥量*/
		 osMutexAcquire(osMutex,osWaitForever);
		osDelay(1000);
  }
}

/*生产者*/
void StartProTask(void *argument)
{
	static uint8_t  i = 0;
	MSGQUEUE_OBJ_t msg;
	uint32_t Probuffer[10] = {1,2,3,4,5,6,7,8,9,10};
	printf("生产者线程\n");
	for(;;)
  {
		for(i = 0 ; i < 10 ; i++)
			msg.Buf[i] = i ;
		osMessageQueuePut(osQueue, &msg, 0U, 0U);
		osDelay(1000);
  }
}

/*消费者*/
void StartConTask(void *argument)
{
	uint8_t  i = 0;
	MSGQUEUE_OBJ_t msg;
  osStatus_t status;
	printf("消费者线程\n");
	for(;;)
  {
		printf("等待队列\n");
    status = osMessageQueueGet(osQueue, &msg, NULL, osWaitForever);   // wait for message
    if (status == osOK) {
			for(i = 0 ; i < 10 ; i++)
				printf("获取队列数据:%d\n",msg.Buf[i]);
			HAL_GPIO_TogglePin(COMPASS_LED_GPIO_Port,COMPASS_LED_Pin);
		}
  }
}
/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
