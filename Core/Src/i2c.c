/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    i2c.c
  * @brief   This file provides code for the configuration
  *          of the I2C instances.
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
#include "i2c.h"

/* USER CODE BEGIN 0 */
#include "../M24SR/m24sr.h"
#include "string.h"
#include "stdio.h"

/* USER CODE END 0 */

I2C_HandleTypeDef hi2c1;

/* I2C1 init function */
void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x00201D2B;//0x2000090E; //0x00201D2B
  hi2c1.Init.OwnAddress1 = 0; //config puts 2 here, we want 0
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

void HAL_I2C_MspInit(I2C_HandleTypeDef* i2cHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(i2cHandle->Instance==I2C1)
  {
  /* USER CODE BEGIN I2C1_MspInit 0 */

  /* USER CODE END I2C1_MspInit 0 */

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**I2C1 GPIO Configuration
    PB6     ------> I2C1_SCL
    PB7     ------> I2C1_SDA
    */
    GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL; //TODO try pull up?
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* I2C1 clock enable */
    __HAL_RCC_I2C1_CLK_ENABLE();

    /* I2C1 interrupt Init */
    HAL_NVIC_SetPriority(I2C1_EV_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(I2C1_EV_IRQn);
  /* USER CODE BEGIN I2C1_MspInit 1 */

  /* USER CODE END I2C1_MspInit 1 */
  }
}

void HAL_I2C_MspDeInit(I2C_HandleTypeDef* i2cHandle)
{

  if(i2cHandle->Instance==I2C1)
  {
  /* USER CODE BEGIN I2C1_MspDeInit 0 */

  /* USER CODE END I2C1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_I2C1_CLK_DISABLE();

    /**I2C1 GPIO Configuration
    PB6     ------> I2C1_SCL
    PB7     ------> I2C1_SDA
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_6);

    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_7);

    /* I2C1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(I2C1_EV_IRQn);
  /* USER CODE BEGIN I2C1_MspDeInit 1 */

  /* USER CODE END I2C1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

uint16_t NFC_IO_ReadMultiple(uint8_t DevAddr, uint8_t *pData, uint16_t Size)
{

    //try retry version:
	HAL_StatusTypeDef status;

    uint8_t retry;
    for (retry = 0; retry < 5; retry++) {
    	    status = HAL_I2C_Master_Receive(&hi2c1, ((M24SR_ADDR << 1) | 0x01), pData, Size, 200);
    	    if (status == HAL_OK) break;
    	    HAL_Delay(5);
    }
    if (status != HAL_OK) {
    	char msg[64];
    	sprintf(msg,
    	"I2C RX FAIL: Dev=0x%02X Err=0x%lX State=%d\r\n",
		((M24SR_ADDR << 1) | 0x01),
    	hi2c1.ErrorCode,
    	hi2c1.State);
    	USART2_PutBuffer((uint8_t *)msg, strlen(msg));
    	LL_mDelay(10);
        return NFC_IO_ERROR_TIMEOUT;
    }



    return NFC_IO_STATUS_SUCCESS;
}



uint16_t NFC_IO_WriteMultiple(uint8_t Addr, uint8_t *pBuffer, uint16_t Length)
{
	HAL_StatusTypeDef status;

	//try retry version:
	uint8_t retry;
	for (retry = 0; retry < 5; retry++) {
	        status = HAL_I2C_Master_Transmit(&hi2c1, ((M24SR_ADDR << 1) | 0x00), pBuffer, Length, 200);
	        if (status == HAL_OK) break;
	        HAL_Delay(5);
	}
	if (status != HAL_OK) {
		char msg[64];
		sprintf(msg,
		"I2C TX FAIL: Dev=0x%02X Err=0x%lX State=%d\r\n",
		((M24SR_ADDR << 1) | 0x00),
		hi2c1.ErrorCode,
		hi2c1.State);
		USART2_PutBuffer((uint8_t *)msg, strlen(msg));
		LL_mDelay(10);
	    return NFC_IO_ERROR_TIMEOUT;
	}

	return NFC_IO_STATUS_SUCCESS;
}






/* USER CODE END 1 */
