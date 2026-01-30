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
#include <stdint.h>

//I2C slave address + R or W bit
//#define M24SR_I2C_WRITE 0xAC //datasheet p. 60 section 7.1
//#define M24SR_I2C_READ  0xAD



#define DEFAULT_PASSWORD_INIT { \
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  \
}

// M24SR I2C commands
#define M24SR_I2C_ADDR_7bit                         0x56
#define M24SR_I2C_READ                              0xAC
#define M24SR_I2C_WRITE                             0xAD


#define M24SR_I2C_RELEASE_TIMEOUT                   41
#define M24SR_KILL_RF_SESSION                       0x52
#define M24SR_GET_I2C_SESSION                       0x26



//#define M24SR_CRC_INIT 0xFFFF

// default I2C timeout in ms; used in user-level API, can be overridden when using low-level methods
#define M24SR_DEFAULT_I2C_TIMEOUT                   5000


void     NFC_IO_Delay(uint32_t Delay);
uint16_t NFC_IO_IsDeviceReady (uint8_t Addr, uint32_t Trials);
void     NFC_IO_ReadState(uint8_t * pPinState);
void NFC_IO_RfDisable(uint8_t PinState);

uint16_t Read_NDEF_From_NFC(uint8_t *ndef_buffer, uint16_t buffer_size, uint16_t *out_ndef_length);
//uint8_t convert_to_NDEF(char *text, uint8_t *ndef);
uint16_t Convert_to_NDEF(char *text, uint8_t *ndef);

uint16_t Write_Joke_to_NFC(uint8_t *ndef_message, uint16_t length);


/* M24SR I2C address (7-bit) */
#define M24SR_ADDR              0x56


uint16_t Convert_to_NDEF(char *text, uint8_t *ndef);
uint16_t Write_Joke_to_NFC(uint8_t *ndef_message, uint16_t length);

#endif /* INC_NFC_IO_H_ */
