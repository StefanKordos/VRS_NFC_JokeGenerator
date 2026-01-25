/*
 * nfc_io.c
 *
 *  Created on: Jan 21, 2026
 *      Author: stefan
 */

#include "i2c.h"
#include "../M24SR/m24sr.h"
//#include "nfc_io.h"

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

void NFC_IO_Delay(uint32_t Delay)
{
    HAL_Delay(Delay);
}

void NFC_IO_ReadState(uint8_t *pPinState)
{
    if (pPinState == NULL) return; //-> null ptr might occur
    *pPinState = (uint8_t)LL_GPIO_IsInputPinSet(GPIOA, LL_GPIO_PIN_6); //our GPO polling pin
}

uint16_t Read_NDEF_From_NFC(uint8_t *ndef_buffer, uint16_t buffer_size, uint16_t *out_ndef_length)
{
	uint16_t status;
	uint8_t length_bytes[2]; //datasheet p.23

	if (!ndef_buffer || !out_ndef_length)
	        return M24SR_ERROR_PARAMETER;

	*out_ndef_length = 0;


	M24SR_KillSession(M24SR_I2C_READ); //if there is an active session -> kill

	status = M24SR_SelectApplication(M24SR_I2C_READ); //application select
	if (status != M24SR_ACTION_COMPLETED)
	    return status;

	status = M24SR_SelectNDEFfile(M24SR_I2C_READ, 0x0001); //we went to read ndef file
	if (status != M24SR_ACTION_COMPLETED)
	    return status;

	//first we read the length bytes ->message lenght
	status = M24SR_ReadBinary(M24SR_I2C_READ, 0x00, 0x02, length_bytes);
	if (status != M24SR_ACTION_COMPLETED)
	    return status;

	uint16_t ndef_length =
	    ((uint16_t)length_bytes[0] << 8) |
	     (uint16_t)length_bytes[1];

	//check if lenght is valid
	if (ndef_length == 0 || ndef_length > buffer_size)
	    return M24SR_ERROR_PARAMETER;

	//read the message
	status = M24SR_ReadBinary(M24SR_I2C_READ, 0x02, ndef_length, ndef_buffer);

	//sucess check
	if (status != M24SR_ACTION_COMPLETED)
	    return status;

	*out_ndef_length = ndef_length; //output lenght

	//end session
	M24SR_Deselect(M24SR_I2C_READ);
	return status;
}

