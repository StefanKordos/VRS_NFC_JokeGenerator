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


// pointer to HAL I2C handle
//I2C_HandleTypeDef* M24SR_hi2c;

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

// M24SR file IDs
#define M24SR_SYSTEM_FILE                           0xE101
#define M24SR_CC_FILE                               0xE103
#define M24SR_NDEF_FILE                             0x0001

// M24SR status codes
//#define M24SR_OK                                    0x9000
#define M24SR_ERR_FILE_OVERFLOW_LE                  0x6280
#define M24SR_ERR_END_OF_FILE_REACHED               0x6282
#define M24SR_ERR_PASSWORD_REQUIRED                 0x6300
#define M24SR_ERR_PASSWORD_INCORRECT                0x63C0
#define M24SR_ERR_UPDATE_FAILED                     0x6581
#define M24SR_ERR_INCORRECT_LENGTH                  0x6700
#define M24SR_ERR_INCOMPATIBLE_CMD                  0x6981
#define M24SR_ERR_SECURITY_NOT_SATISFIED            0x6982
#define M24SR_ERR_REFERENCE_UNUSABLE                0x6984
#define M24SR_ERR_INCORRECT_LE_LC                   0x6A80
#define M24SR_ERR_FILE_APP_NOT_FOUND                0x6A82
#define M24SR_ERR_FILE_OVERFLOW_LC                  0x6A84
#define M24SR_ERR_INCORRECT_P1_P2                   0x6A86
#define M24SR_ERR_INCORRECT_INS                     0x6D00
#define M24SR_ERR_INCORRECT_CLA                     0x6E00

// custom status codes
#define M24SR_ERR_I2C_TIMEOUT                       0xFF00
#define M24SR_ERR_CRC_MISMATCH                      0xFF01
#define M24SR_ERR_WRITE_FAILED                      0xFF02

// M24SR file and field lengths
#define M24SR_SYSTEM_FILE_LENGTH                    0x12
#define M24SR_UID_LENGTH                            0x07
#define M24SR_CC_FILE_LENGTH                        0x0F

// initial CRC-A value
#define M24SR_CRC_INIT                              0x6363
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


unsigned short M24SR_ReadTag(unsigned char* buf);

//--------------alternative driver-------------------
/* M24SR I2C address (7-bit) */
#define M24SR_ADDR              0x56

/* Status codes */
typedef enum {
    M24SR_OK = 0,
    M24SR_ERROR,
    M24SR_TIMEOUT,
    M24SR_NACK
} M24SR_Status;

/* Command status bytes */
#define M24SR_STATUS_OK         0x9000
#define M24SR_STATUS_EOF        0x6282

/* File IDs */
#define M24SR_CC_FILE_ID        0xE103
#define M24SR_NDEF_FILE_ID      0x0001
#define M24SR_SYS_FILE_ID       0xE101

M24SR_Status M24SR_KillRFSession(void);

M24SR_Status M24SR_Deselect_alt(void);

M24SR_Status M24SR_SelectApplication_alt(void);

M24SR_Status M24SR_SelectCCFile_alt(void);

M24SR_Status M24SR_SelectNDEFFile_alt(void);

M24SR_Status M24SR_ReadBinary_alt(uint16_t offset, uint8_t length, uint8_t *pData);

M24SR_Status M24SR_ReadNDEFLength_alt(uint16_t *pLength);

M24SR_Status M24SR_ReadNDEF_alt(uint8_t *pData, uint16_t maxLen, uint16_t *pActualLen);






#endif /* INC_NFC_IO_H_ */
