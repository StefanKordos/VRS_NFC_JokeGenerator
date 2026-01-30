/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "main.h"
#include "dma.h"
#include "i2c.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include <stdio.h>
#include <string.h>
#include <nfc_io.h>

//this include to separate .h
//#include "../M24SR/m24sr.h"



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

/* USER CODE BEGIN PV */
static uint8_t ndefBuffer[256];
static uint8_t uidBuffer[7];


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
static void PrintHex(const uint8_t *data, uint16_t len)
{
    char buf[8];

    for (uint16_t i = 0; i < len; i++)
    {
        sprintf(buf, "%02X ", data[i]);
        USART2_PutBuffer((uint8_t *)buf, strlen(buf));

        if ((i + 1) % 16 == 0)
        {
            USART2_PutBuffer((uint8_t *)"\r\n", 2);
        }
    }

    if (len % 16 != 0)
    {
        USART2_PutBuffer((uint8_t *)"\r\n", 2);
    }
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_I2C1_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
  char formatted_string[100];
  char msg[128];
  uint16_t status = 0;


  M24SR_Init(M24SR_I2C_READ, M24SR_GPO_POLLING); //M24SR_WAITINGTIME_POLLING


  status = M24SR_KillSession(M24SR_I2C_READ);
  sprintf(msg, "M24SR found. Kill session status: 0x%04X\r\n", status);
  USART2_PutBuffer((uint8_t *)msg, strlen(msg));
  LL_mDelay(50);



  //datasheet p. 24
  //M24SR_ManageRFGPO(M24SR_I2C_READ, 1);


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */


  while (1)
  {

	  //sprintf(formatted_string, "test\n\r\0");
	  //USART2_PutBuffer(formatted_string, strlen(formatted_string));
	  //LL_mDelay(100);
	  //LL_GPIO_TogglePin(GPIOB, LL_GPIO_PIN_3);
	  //LL_mDelay(200);

	  //i2c read test output send
	  /*for (uint16_t i = 0; i < ndef_len; i++)
	  {
	      char hex[5];
	      sprintf(hex, "%02X ", ndef_raw[i]);
	      USART2_PutBuffer((uint8_t *)hex, strlen(hex));
	  }
	  USART2_PutBuffer((uint8_t *)"\r\n", 2);

	  LL_mDelay(5000);*/

	  //NFC_IO_ReadState(&state);
	  //USART2_PutBuffer((uint8_t *)"NFC alive\r\n", 11);
	  //LL_mDelay(100);

	  /*if (LL_GPIO_IsInputPinSet(GPIOA, LL_GPIO_PIN_6))
	          USART2_PutBuffer((uint8_t *)"GPO=HIGH\r\n", 10);
	      else
	          USART2_PutBuffer((uint8_t *)"GPO=LOW\r\n", 9);

	      LL_mDelay(100);*/


	  uint8_t ndef_raw[256];
	  uint16_t ndef_len = 0;

	    //i2c read test
	    LL_mDelay(100);
	    status = Read_NDEF_From_NFC(ndef_raw, sizeof(ndef_raw), &ndef_len);
	    LL_mDelay(100);


	    //char msg[64];
	    sprintf(msg, "Read status: 0x%04X, len: %u\r\n", status, ndef_len);
	    USART2_PutBuffer((uint8_t *)msg, strlen(msg));
	    LL_mDelay(20);
	    USART2_PutBuffer("\r\n", 2);
	    LL_mDelay(20);
	    PrintHex(ndef_raw, ndef_len);
	    LL_mDelay(20);
	    USART2_PutBuffer("\r\n", 2);
	    LL_mDelay(20);
	    uint8_t langLen = ndef_raw[4] & 0x3F;
	    USART2_PutBuffer(&ndef_raw[5 + langLen], ndef_raw[2] - 1 - langLen);

	    LL_mDelay(3000000);




    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  LL_FLASH_SetLatency(LL_FLASH_LATENCY_0); //0
  while(LL_FLASH_GetLatency()!= LL_FLASH_LATENCY_0) //0
  {
  }
  LL_RCC_HSI_Enable();

   /* Wait till HSI is ready */
  while(LL_RCC_HSI_IsReady() != 1)
  {

  }
  LL_RCC_HSI_SetCalibTrimming(16);
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1); //1
  LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_HSI); //HSI

   /* Wait till System clock is ready */
  while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_HSI) //HSI
  {

  }
  LL_SetSystemCoreClock(8000000);  //8000000

   /* Update the time base */
  if (HAL_InitTick (TICK_INT_PRIORITY) != HAL_OK)
  {
    Error_Handler();
  }
  LL_RCC_SetI2CClockSource(LL_RCC_I2C1_CLKSOURCE_HSI);
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
