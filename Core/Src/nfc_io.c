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

