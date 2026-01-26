/*
 * nfc_io.h
 *
 *  Created on: Jan 21, 2026
 *      Author: stefan
 */

#ifndef INC_NFC_IO_H_
#define INC_NFC_IO_H_

#include "../M24SR/m24sr.h"
#include "stm32f3xx_ll_i2c.h"
#include "stm32f3xx_ll_gpio.h"

//I2C slave address + R or W bit
#define M24SR_I2C_WRITE 0xAC //datasheet p. 60 section 7.1
#define M24SR_I2C_READ  0xAD

void     NFC_IO_Delay(uint32_t Delay);
uint16_t NFC_IO_IsDeviceReady (uint8_t Addr, uint32_t Trials);
void     NFC_IO_ReadState(uint8_t * pPinState);

uint16_t Read_NDEF_From_NFC(uint8_t *ndef_buffer, uint16_t buffer_size, uint16_t *out_ndef_length);

uint8_t Convert_to_NDEF(char *text, uint8_t *ndef);
uint16_t Write_Joke_to_NFC(uint8_t *ndef_message, uint16_t length);

#endif /* INC_NFC_IO_H_ */
