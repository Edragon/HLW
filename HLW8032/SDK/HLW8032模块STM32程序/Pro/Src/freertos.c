/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * Copyright (c) 2018 STMicroelectronics International N.V. 
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"

/* USER CODE BEGIN Includes */     
#include "stdio.h"
#include "usart.h"
#include "string.h"

unsigned char a = 10;
unsigned char b = 0;

#define IO_Control_Relay(x,y)  IO_Control_##x(y)
#define IO_Control_1(y)    (a = y)
#define IO_Control_2(y)    (a = y + 1)

/* USER CODE END Includes */

/* Variables -----------------------------------------------------------------*/
osThreadId defaultTaskHandle;

/* USER CODE BEGIN Variables */
osThreadId lpUart1ScanTaskHandle;
xSemaphoreHandle xScanSemaphore;

extern uint8_t ReceiveBuff[RXBUFFERSIZE];
extern uint8_t ReceiveInfo[RXBUFFERSIZE];
extern uint8_t Rx_BufferLen;
extern uint8_t Rx_InfoLen;
/* USER CODE END Variables */

/* Function prototypes -------------------------------------------------------*/
void StartDefaultTask(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* USER CODE BEGIN FunctionPrototypes */
void StartlpUart1ScanTask(void const * argument);

/* USER CODE END FunctionPrototypes */

/* Hook prototypes */

/* Init FreeRTOS */

void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
       
  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  xScanSemaphore = xSemaphoreCreateBinary();
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 128);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  osThreadDef(usartTask, StartlpUart1ScanTask, osPriorityHigh, 0, 128);
  lpUart1ScanTaskHandle = osThreadCreate(osThread(usartTask), NULL);  
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */
}

/* StartDefaultTask function */
void StartDefaultTask(void const * argument)
{

  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(500);
    /*
    printf("Enter Default Task !\r\n");
    IO_Control_Relay(1,b);
    printf("a is %d\r\n",a);
    a = 10;
    IO_Control_Relay(2,b);
    printf("a is %d\r\n",a);
    a = 10;
    b++;
    if(b > 255)
    {
      b = 0;
    }
    */
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Application */
void StartlpUart1ScanTask(void const * argument)
{
  uint32_t temp = 0;
  float VoltValue = 0.0;
  float IcurrentValue = 0.0;
  configASSERT(xScanSemaphore);
  xSemaphoreTake(xScanSemaphore,0);
  for(;;)
  {
    xSemaphoreTake(xScanSemaphore,portMAX_DELAY);
    printf("\r\n rx_len = %02d\r\n",Rx_InfoLen);
    HAL_UART_Transmit(&huart3,ReceiveInfo,Rx_InfoLen,0x0F);
    if(0 != Rx_InfoLen)
    {
      printf("\r\n");
      printf("State REG is %X\r\n",ReceiveInfo[0]);
      printf("Check REG is %X\r\n",ReceiveInfo[1]);
      
      printf("Voltage parameter REG is %X %X %X \r\n",ReceiveInfo[2],ReceiveInfo[3],ReceiveInfo[4]);
      temp = (ReceiveInfo[5]<<16) + (ReceiveInfo[6]<<8) + ReceiveInfo[7];
      printf("Voltage REG is %X %X %X, and Voltage Value is %d\r\n",ReceiveInfo[5],ReceiveInfo[6],ReceiveInfo[7],temp);
      temp = 0;
      
      printf("Current parameter REG is %X %X %X \r\n",ReceiveInfo[8],ReceiveInfo[9],ReceiveInfo[10]);
      temp = (ReceiveInfo[11]<<16) + (ReceiveInfo[12]<<8) + ReceiveInfo[13];
      printf("Current REG is %X %X %X, and Current Value is %d\r\n",ReceiveInfo[11],ReceiveInfo[12],ReceiveInfo[13],temp);
      temp = 0;
      
      printf("Power parameter REG is %X %X %X \r\n",ReceiveInfo[14],ReceiveInfo[15],ReceiveInfo[16]);
      temp = (ReceiveInfo[17]<<16) + (ReceiveInfo[18]<<8) + ReceiveInfo[19];
      printf("Power REG is %X %X %X, and Power Value is %d\r\n",ReceiveInfo[17],ReceiveInfo[18],ReceiveInfo[19],temp);
      temp = 0;
      
      printf("Data Updata REG is %X\r\n",ReceiveInfo[20]);
      printf("PF REG is %X %X\r\n",ReceiveInfo[21],ReceiveInfo[22]);
      printf("CheckSumREG is %X\r\n",ReceiveInfo[23]);
   }
    memset(ReceiveInfo,0x00,Rx_InfoLen);
    Rx_InfoLen = 0;
  }
}
/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
