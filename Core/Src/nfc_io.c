/*
 * nfc_io.c
 *
 *  Created on: Jan 21, 2026
 *      Author: stefan
 */

#include "i2c.h"
#include "usart.h"
#include "../M24SR/m24sr.h"
#include "nfc_io.h"
#include "stdlib.h"
#include "string.h"
#include "stdio.h"
#include "stm32f3xx_hal.h"



uint8_t DefaultPassword[16] = DEFAULT_PASSWORD_INIT;

#define M24SR_DEVICE_ADDR_W     ((M24SR_ADDR << 1) | 0x00)
#define M24SR_DEVICE_ADDR_R     ((M24SR_ADDR << 1) | 0x01)



uint16_t NFC_IO_IsDeviceReady(uint8_t Addr, uint32_t Trials)
{
	//simpler version
	for (uint32_t i = 0; i < Trials; i++)
	    {
	        if (HAL_I2C_IsDeviceReady(&hi2c1, Addr, 500, 3000) == HAL_OK)
	        {
	            return NFC_IO_STATUS_SUCCESS;
	        }
	}

	return NFC_IO_ERROR_TIMEOUT;
}


void NFC_IO_RfDisable(uint8_t PinState){
    if (PinState){
        LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_7);
    }
    else{
        LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_7);
    }
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
	uint16_t step = 0;
	char msg[64];
	uint8_t length_bytes[2]; //datasheet p.23

	if (!ndef_buffer || !out_ndef_length)
	    return M24SR_ERROR_PARAMETER;

	*out_ndef_length = 0;

	//opens session
	status = M24SR_KillSession(M24SR_I2C_READ);
	HAL_Delay(10);
	uint8_t counter = 1;
	sprintf(msg, "Kill session status = 0x%04X\r\n", status); //to check if session open
	USART2_PutBuffer((uint8_t *)msg, strlen(msg));
	LL_mDelay(20);


	status = M24SR_SelectApplication(M24SR_I2C_READ); //application select
	//HAL_Delay(10);
	counter = 2;
	sprintf(msg, "Select Aplication status = 0x%04X\r\n", status);
	USART2_PutBuffer((uint8_t *)msg, strlen(msg));
	LL_mDelay(20);


	status = M24SR_SelectCCfile(M24SR_I2C_READ);
	counter = 3;
	sprintf(msg, "Select CCfile status = 0x%04X\r\n", status);
	USART2_PutBuffer((uint8_t *)msg, strlen(msg));
	LL_mDelay(20);


/*
	uint8_t cc[15];   //CC file is typc. 15 bytes
	status = M24SR_ReadBinary(M24SR_I2C_READ, 0x0000, sizeof(cc), cc);
	counter = 4;
	sprintf(msg, "stat = 0x%04X\r\n", status);
	USART2_PutBuffer((uint8_t *)msg, strlen(msg));
	LL_mDelay(20);*/



	status = M24SR_SelectNDEFfile(M24SR_I2C_READ, 0x0001);
	counter = 5;
	sprintf(msg, "Select NDEF file status = 0x%04X\r\n", status);
	USART2_PutBuffer((uint8_t *)msg, strlen(msg));
	LL_mDelay(20);

	//------selecting message done----------------------------

	uint8_t nlenBuf[2];
	status =  M24SR_ReadBinary(M24SR_I2C_READ, 0x0000, 2, nlenBuf);
	uint16_t nlen = (nlenBuf[0] << 8) | nlenBuf[1];
	*out_ndef_length = nlen;
	sprintf(msg, "Read lenght status = 0x%04X\r\n", status);
    USART2_PutBuffer((uint8_t *)msg, strlen(msg));
    LL_mDelay(20);
	counter = 7;


	//read payload
	//uint8_t ndef[256];  // must be >= nlen
	status = M24SR_ReadBinary(M24SR_I2C_READ, 0x0002, nlen, ndef_buffer);
	counter = 8;
	sprintf(msg, "Read payload status = 0x%04X\r\n", status);
	USART2_PutBuffer((uint8_t *)msg, strlen(msg));
	LL_mDelay(20);

    //we should be done
    status = M24SR_Deselect(M24SR_I2C_READ);
    sprintf(msg, "Deselect status = 0x%04X\r\n", status);
    USART2_PutBuffer((uint8_t *)msg, strlen(msg));
    LL_mDelay(20);

    M24SR_KillSession(M24SR_I2C_READ);
    LL_mDelay(20);




	return status;
}

uint16_t Convert_to_NDEF(char *text, uint8_t *ndef)
{
	if (text == NULL || ndef == NULL)
		return 1;

	/*Byte 0–1  → NLEN
	Byte 2    → Flags
	Byte 3    → Type Length
	Byte 4    → Payload Length
	Byte 5    → Type ('T')
	Byte 6    → Status byte
	Byte 7–8  → Language ("en")
	Byte 9+   → Text
	*/

	uint16_t text_len = strlen(text);

	//Status byte 1B, Language code 2B "en", text text_lenB
	uint16_t payload_length = 1 + 2 + text_len; //NFCForum-TS-RTD_Text_1.0 p.4

	if (payload_length > 255)
	{
		printf("[NDEF ERROR] Payload length %d exceeds SR limit (255)!\r\n", payload_length);
		return 0;
	}

	//Length of NDEF record
	/*
	 *flags(MB ME CF SR IL TNF) 1B
	 *TYPE LENGTH 1B
	 *PAYLOAD_LENGTH (SR=1 -> 1 byte, NDEF.pdf 3.2.4) enough for joke
	 *TYPE 1B
	 *PAYLOAD payload_lengthB*/
	uint16_t ndef_record_length = 4 + payload_length; //NFCForum-TS-NDEF_1.0 p.14

	// NLEN (2B) – NDEF message length in big-endian (Type 4 Tag, M24SR64-Y p.23)
	//MSB first NFCForum-TS-NDEF_1.0 p.13
	//this is not part of NDEF record
	ndef[0] = (ndef_record_length >> 8) & 0xFF; //MSB shifting to right
	ndef[1] = (ndef_record_length ) & 0xFF; //LSB

	//-------- NDEF RECORD NFCForum-TS-NDEF_1.0 s.3.2.4--------
	/*
	 * MB=1, ME=1, CF=0, IL=0
	 * SR=1 -> Short Record
	 * TNF=001 -> Well Known Type*/
	ndef[2] = 0xD1; //11010001

	//-------- Text record SR(Short Record) NFCForum-TS-NDEF_1.0 s.3.2 --------
	/*
	 * TYPE_LENGTH = 1
	 * (single character 'T')
	 */
	ndef[3] = 0x01;

	/*
	 * PAYLOAD_LENGTH (SR=1 → 1 byte)
	 */
	ndef[4] = payload_length;

	/*
	 * TNF=0x01 → RTD type namespace
	 */
	ndef[5] = 0x54;  // ASCII 'T'

	//-------- PAYLOAD NFCForum-TS-NDEF_1.0 s.3.2--------
	/*
	 * Payload = Status Byte + Language Code + Text
	 * Bit number (0 is LSB)
	 * Status byte:
	 * Bit 7 = 0 → UTF-8 encoding
	 * Bits 5..0 = language length (2 bytes → "en")
	 *
	 * Defined in NFC RTD Text specification
	 */

	//bit7  bit6  bit5..0
	// 0     0      000010 2B for "en"
	ndef[6] = 0x02;

	//Language Code
	ndef[7] = 'e';
	ndef[8] = 'n';

	memcpy(&ndef[9], text, text_len); //copy length of bytes without \0
	return 2 + ndef_record_length; //2-byte "Length Header"
}

uint16_t Write_Joke_to_NFC(uint8_t *ndef_message, uint16_t length)
{
	uint16_t status;
	char msg[64];

	//Device select 0xAC: to send a request to the M24SR64-Y m24sr64-Y s.7.9
	M24SR_KillSession(M24SR_DEVICE_ADDR_W); //if there is an active session -> kill

	status = M24SR_SelectApplication(M24SR_DEVICE_ADDR_W); //select NDEF tag application m24sr64-y s.8.9
	if (status != M24SR_ACTION_COMPLETED)
		return status;

	status = M24SR_SelectNDEFfile(M24SR_DEVICE_ADDR_W, 0x0001); //select NDEF file m24sr64-y s. 3.1.1
	if (status != M24SR_ACTION_COMPLETED)
		return status;

	USART2_PutBuffer("file selected\r\n", strlen("file selected\r\n"));
	LL_mDelay(20);

	status = M24SR_UpdateBinary(
			M24SR_DEVICE_ADDR_W,
				0x0000, //beginning of file
				length, // total length (NLEN + record)
				ndef_message
			);

	sprintf(msg, "updateBinary status = 0x%04X\r\n", status); //to check if session open
	USART2_PutBuffer((uint8_t *)msg, strlen(msg));
	LL_mDelay(20);

	if (status != NFC_IO_STATUS_SUCCESS){
		USART2_PutBuffer("write fail\r\n", strlen("write fail\r\n"));
		LL_mDelay(20);
		//return status;
	}

	//USART2_PutBuffer("write deselect not done\r\n", strlen("write deselect not done\r\n"));
	//LL_mDelay(20);

	//end of communication
	M24SR_Deselect(M24SR_DEVICE_ADDR_W);
	USART2_PutBuffer("write deselect done\r\n", strlen("write deselect done\r\n"));
	LL_mDelay(50);
	return status;
}



