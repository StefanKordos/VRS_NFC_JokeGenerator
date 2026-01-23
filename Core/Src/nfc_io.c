/*
 * nfc_io.c
 *
 *  Created on: Jan 21, 2026
 *      Author: stefan
 */

#include "i2c.h"
#include "../M24SR/m24sr.h"

uint16_t NFC_IO_IsDeviceReady(uint8_t Addr, uint32_t Trials)
{
    //uint8_t i2c_addr = Addr >> 1;
	uint8_t i2c_addr = Addr;

	uint32_t start = HAL_GetTick();
	uint32_t timeout_ms = 1500;

	//use polling -> HAL_I2C_IsDeviceReady() locks? during RF
	while ((HAL_GetTick() - start) < timeout_ms)
	{
	    if (HAL_I2C_IsDeviceReady(&hi2c1, i2c_addr, 1, 100) == HAL_OK)
	        return NFC_IO_STATUS_SUCCESS;
	}

	return NFC_IO_ERROR_TIMEOUT;
}
