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
  //char formatted_string[100];
  char msg[128];
  uint16_t status = 0;


  //I2C init + GPO polling
  //M24SR_Init(M24SR_I2C_READ, M24SR_GPO_POLLING); //don't do init for now
/*
  sprintf(msg, "before init\r\n");
  USART2_PutBuffer((uint8_t *)msg, strlen(msg));
  LL_mDelay(100);*/

  M24SR_Init(M24SR_I2C_READ, M24SR_GPO_POLLING); //M24SR_WAITINGTIME_POLLING
  //M24SR_ManageRFGPO(M24SR_I2C_READ, 1);


  status = M24SR_KillSession(M24SR_I2C_READ);
  sprintf(msg, "M24SR found. Kill session status: 0x%04X\r\n", status);
  USART2_PutBuffer((uint8_t *)msg, strlen(msg));
  LL_mDelay(50);



  //datasheet p. 24
  //M24SR_ManageRFGPO(M24SR_I2C_READ, 1);
/*
  //uint8_t state;
  //--------------------------Rado ndef_convert test-------------------------------
  char myStaticJoke[] = "This is a static test joke";
    uint8_t ndefBuffer[300];
    uint16_t ndefLen;

    ndefLen = Convert_to_NDEF(myStaticJoke, ndefBuffer);

      if (ndefLen > 0) {
          USART2_PutBuffer((uint8_t*)"NDEF Conversion Success!\r\n", 26);
          LL_mDelay(100);

          for(int i=0; i<ndefLen; i++) {
              sprintf(formatted_string, "%02X ", ndefBuffer[i]);
              USART2_PutBuffer((uint8_t*)formatted_string, strlen(formatted_string));
              LL_mDelay(100);
          }
          USART2_PutBuffer((uint8_t*)"\r\n", 2);
          LL_mDelay(100);
      } else {
          USART2_PutBuffer((uint8_t*)"NDEF Failed (Too long?)\r\n", 25);
          LL_mDelay(100);
      }
//--------------------------------------------------------------------------------------*/

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  /*uint8_t gpo;
  char msg_gpo[64];
  NFC_IO_ReadState(&gpo);
  sprintf(msg_gpo, "GPO=%d\r\n", gpo);
  USART2_PutBuffer((uint8_t *)msg_gpo, strlen(msg_gpo));
  LL_mDelay(200);*/
  //LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_7); //RF to gnd
/*
  uint16_t status = NFC_IO_IsDeviceReady(M24SR_I2C_READ, 3);

  if (status == NFC_IO_STATUS_SUCCESS)
  {
      USART2_PutBuffer((uint8_t *)"M24SR READY\r\n", 13);
      LL_mDelay(100);
  }
  else
  {
      USART2_PutBuffer((uint8_t *)"M24SR NOT READY\r\n", 18);
      LL_mDelay(100);
  }*/

/*
  uint8_t ndef_raw[256];
  uint16_t ndef_len = 0;

  //i2c read test
  LL_mDelay(100);
  status = Read_NDEF_From_NFC(ndef_raw, sizeof(ndef_raw), &ndef_len);
  LL_mDelay(100);


  //char msg[64];
  sprintf(msg, "Read status: 0x%04X, len: %u\r\n", status, ndef_len);
  USART2_PutBuffer((uint8_t *)msg, strlen(msg));
  LL_mDelay(500);*/


  //unsigned char buf[256];
  //unsigned short state = M24SR_ReadTag(buf);
  //uint16_t state = stm32_read_ndef_text(buf, 200);

/*
  uint8_t payload[] = {
      0xD1, 0x01, 0x0F, 0x54,
      0x02, 'e','n',
      'H','e','l','l','o'
  };

  uint16_t len = sizeof(payload);

  uint16_t state = Write_Ndef(&payload, sizeof(payload));
  LL_mDelay(150);



  sprintf(msg, "write status: 0x%04X\r\n", state);
  USART2_PutBuffer((uint8_t *)msg, strlen(msg));
  LL_mDelay(100);*/


  /*sprintf(msg, "NLEN raw: %02X %02X\r\n",
          length_bytes[0], length_bytes[1]);
  USART2_PutBuffer((uint8_t *)msg, strlen(msg));
  LL_mDelay(500);*/

  //alternate driver experiment-----------------------------------
/*M24SR_Status nfcStatus;

  HAL_Delay(100);
  USART2_PutBuffer((uint8_t *)"\r\n\r\n=== X-NUCLEO-NFC01A1 Test ===\r\n", strlen("\r\n\r\n=== X-NUCLEO-NFC01A1 Test ===\r\n"));
  LL_mDelay(100);
  //Test I2C connection
  nfcStatus = M24SR_KillRFSession();
  if (nfcStatus != M24SR_OK) {
      USART2_PutBuffer((uint8_t *)"ERROR: M24SR not responding on I2C!\r\n", strlen("ERROR: M24SR not responding on I2C!\r\n"));
      LL_mDelay(100);
  } else {
      USART2_PutBuffer((uint8_t *)"M24SR found on I2C bus.\r\n", strlen("M24SR found on I2C bus.\r\n"));
      LL_mDelay(100);
  }  */


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


	  //-------------------alternate driver experiment----------------------
/*
	  uint16_t ndefLen = 0;

	  USART2_PutBuffer((uint8_t *)"\r\n--- Reading NDEF ---\r\n", strlen("\r\n--- Reading NDEF ---\r\n"));

	  //fresh session
	  M24SR_KillRFSession();

	  // Select NFC application
	  nfcStatus = M24SR_SelectApplication_alt();
	  if (nfcStatus != M24SR_OK) {
	      USART2_PutBuffer((uint8_t *)"ERROR: SelectApplication failed\r\n", strlen("ERROR: SelectApplication failed\r\n"));
	      HAL_Delay(2000);
	      continue;
	   }

	  nfcStatus = M24SR_SelectCCFile_alt();
	  if (nfcStatus != M24SR_OK) {
	      USART2_PutBuffer((uint8_t *)"ERROR: SelectCCFile failed\r\n", strlen("ERROR: SelectCCFile failed\r\n"));
	      HAL_Delay(2000);
	      continue;
	      }
	  USART2_PutBuffer((uint8_t *)"CC file selected\r\n", strlen("CC file selected\r\n"));
	  HAL_Delay(20);

	 nfcStatus = M24SR_SelectNDEFFile_alt();
      if (nfcStatus != M24SR_OK) {
	      USART2_PutBuffer((uint8_t *)"ERROR: SelectNDEFFile failed\r\n", strlen("ERROR: SelectNDEFFile failed\r\n"));
	      HAL_Delay(2000);
	      continue;
	  }
	  USART2_PutBuffer((uint8_t *)"NDEF file selected\r\n", strlen("NDEF file selected\r\n"));
	  HAL_Delay(20);

	  //Read NDEF message
	  nfcStatus = M24SR_ReadNDEF_alt(ndefBuffer, sizeof(ndefBuffer), &ndefLen);
	  if (nfcStatus == M24SR_OK && ndefLen > 0) {
	      //Debug_Printf("NDEF (%u bytes):\r\n", ndefLen);
		  sprintf(msg, "NDEF (%d bytes):\r\n", ndefLen);
		  USART2_PutBuffer((uint8_t *)msg, strlen(msg));
	      //USART2_PutBuffer("NDEF (%d bytes):\r\n", ndefLen);
	      HAL_Delay(30);
	      PrintHex(ndefBuffer, ndefLen);
	      HAL_Delay(10);

	      //Parse text record if present
	      if (ndefLen > 7 && (ndefBuffer[0] & 0x07) == 0x01) {
	          uint8_t typeLen = ndefBuffer[1];
	           uint8_t payloadLen = ndefBuffer[2];

	              if (typeLen == 1 && ndefBuffer[3] == 'T' && payloadLen > 1) {
	                  uint8_t langLen = ndefBuffer[4] & 0x3F;
	                  if (5 + langLen + (payloadLen - 1 - langLen) <= ndefLen) {
	                	  USART2_PutBuffer("\r\nText: ", 8);
	                	  HAL_Delay(10);
	                      //HAL_UART_Transmit(&huart2, &ndefBuffer[5 + langLen], payloadLen - 1 - langLen, 100);
	                      USART2_PutBuffer(&ndefBuffer[5 + langLen], payloadLen - 1 - langLen);
	                      HAL_Delay(40);
	                      USART2_PutBuffer("\r\n", 2);
	                      HAL_Delay(10);
	                  }
	              }
	          }
	      } else {
	          //Debug_Print("No NDEF data or read error\r\n");
	          USART2_PutBuffer((uint8_t *)"No NDEF data or read error\r\n", strlen("No NDEF data or read error\r\n"));
	          HAL_Delay(20);
	      }








	  // Close session
	  nfcStatus = M24SR_Deselect_alt();
	  if (nfcStatus != M24SR_OK) {
	  	  USART2_PutBuffer((uint8_t *)"ERROR: session not closed\r\n", strlen("ERROR: session not closed\r\n"));
	  	  HAL_Delay(2000);
	  	  continue;
	  }
	  USART2_PutBuffer((uint8_t *)"session closed\r\n", strlen("session closed\r\n"));


	  HAL_Delay(300000);                    */

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
