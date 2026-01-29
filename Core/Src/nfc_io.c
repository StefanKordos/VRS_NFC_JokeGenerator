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


/*
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
}*/

uint16_t NFC_IO_IsDeviceReady(uint8_t Addr, uint32_t Trials)
{

	/*HAL_StatusTypeDef status;
    uint32_t tickstart = 0;
    uint32_t currenttick = 0;

    tickstart = HAL_GetTick();

    status = HAL_I2C_IsDeviceReady(&hi2c1, Addr, Trials, NFC_I2C_TIMEOUT);

    do
    {
        status = HAL_I2C_IsDeviceReady(&hi2c1, Addr, Trials, 1000);
        currenttick = HAL_GetTick();
    } while( ( (currenttick - tickstart) < 1500) && (status != HAL_OK) );


    if (status != HAL_OK)
    {
        return NFC_IO_ERROR_TIMEOUT;
    }

    return NFC_IO_STATUS_SUCCESS;*/

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


/*
uint16_t NFC_IO_IsDeviceReady(uint8_t DevAddr, uint32_t Trials)
{
    if (HAL_I2C_IsDeviceReady(
            &hi2c1,
            DevAddr ,
            Trials,
            HAL_MAX_DELAY
        ) != HAL_OK)
    {
    	char msg[64];
    	    	                sprintf(msg,
    	    	                "IO Device not ready: Dev=0x%02X Err=0x%lX State=%d\r\n",
    	    					DevAddr,
    	    	                hi2c1.ErrorCode,
    	    	                hi2c1.State);
    	    	                USART2_PutBuffer((uint8_t *)msg, strlen(msg));
    	    	                LL_mDelay(150);

        return NFC_IO_STATUS_ERROR;
    }

    return NFC_IO_STATUS_SUCCESS;
}*/

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

	//M24SR_RFConfig(1); disable RF


	//opens session
	status = M24SR_GetSession(M24SR_I2C_READ); //-> helped increasing trials and timeout in isdeviceready
	HAL_Delay(10);
	uint8_t counter = 1;
	sprintf(msg, "stat = %X\r\n", status); //to check if session open
	USART2_PutBuffer((uint8_t *)msg, strlen(msg));
	//LL_mDelay(10);


	status = M24SR_SelectApplication(M24SR_I2C_READ); //application select
	//HAL_Delay(10);
	counter = 2;
	sprintf(msg, "stat = %X\r\n", status);
	USART2_PutBuffer((uint8_t *)msg, strlen(msg));
	LL_mDelay(10);


	status = M24SR_SelectCCfile(M24SR_I2C_READ);
	counter = 3;
	sprintf(msg, "stat = %X\r\n", status);
	USART2_PutBuffer((uint8_t *)msg, strlen(msg));
	LL_mDelay(10);



	uint8_t cc[15];   //CC file is typc. 15 bytes
	status = M24SR_ReadBinary(M24SR_I2C_READ, 0x0000, sizeof(cc), cc);
	counter = 4;
	sprintf(msg, "stat = %X\r\n", status);
	USART2_PutBuffer((uint8_t *)msg, strlen(msg));
	LL_mDelay(10);



	status = M24SR_SelectNDEFfile(M24SR_I2C_READ, 0x0001);
	counter = 5;
	sprintf(msg, "stat = %X\r\n", status);
	USART2_PutBuffer((uint8_t *)msg, strlen(msg));
	LL_mDelay(10);

	//------selecting message done----------------------------

	uint8_t nlenBuf[2];
	status =  M24SR_ReadBinary(M24SR_I2C_READ, 0x0000, 2, nlenBuf);
	uint16_t nlen = (nlenBuf[0] << 8) | nlenBuf[1];
	sprintf(msg, "stat = %X\r\n", status);
    USART2_PutBuffer((uint8_t *)msg, strlen(msg));
    LL_mDelay(10);
	counter = 7;


	//read payload
	uint8_t ndef[256];  // must be >= nlen
	status = M24SR_ReadBinary(M24SR_I2C_READ, 0x0002, nlen, ndef);
	counter = 8;
	sprintf(msg, "stat = %X\r\n", status);
	USART2_PutBuffer((uint8_t *)msg, strlen(msg));
	LL_mDelay(10);

    //we should be done
    M24SR_Deselect(M24SR_I2C_READ);
    M24SR_KillSession(M24SR_I2C_READ);

	return status;
}


//rado code

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







/*

uint8_t convert_to_NDEF(char *text, uint8_t *ndef){
    char msg[100];

	if (text == 0)
		return 1;

	size_t msg_size = strlen(text)+11;
	uint8_t P1 = (uint8_t) ((msg_size & 0xFF00 )>>8);
	uint8_t P2 = (uint8_t) (msg_size & 0x00FF );

	size_t msg_last = strlen(text)+4;


	uint8_t TNF_FLAGS = 0xC1;
	uint8_t type_length = 0x1;
	uint8_t payload_length = (uint8_t)(msg_size + 3);
	uint8_t type_text = 0x54;
	uint8_t status = 0x02;
	uint8_t language1 = 0x65;
	uint8_t language2 = 0x6E;

	uint8_t ndef_header[] = {P1, P2, TNF_FLAGS, type_length,  0x0 ,0x0,0x0,msg_last, type_text, status, language1, language2};

    memcpy(ndef, ndef_header, sizeof(ndef_header));
    memcpy(ndef + sizeof(ndef_header), text, strlen(text)+1);


	return 0;
}
*/





uint16_t Write_Ndef(uint8_t *ndef_message, uint16_t total_lenght)
{
    char msg2[200];
    uint16_t status;
    uint8_t counter = 0;

    uint8_t zero[2] = {0, 0};
    uint8_t lenBuf[2] = { total_lenght >> 8, total_lenght & 0xFF };

    // Start I2C communication
    M24SR_GetSession(M24SR_I2C_WRITE);

    char msg[64];
    counter = 1;
        sprintf(msg, "cnt= %d\r\n", counter);
        USART2_PutBuffer((uint8_t *)msg, strlen(msg));
        LL_mDelay(10);
/*
        uint8_t get_i2c_session = 0x52;
            HAL_I2C_Master_Transmit(&hi2c1, M24SR_I2C_ADDR << 1,
                                    &get_i2c_session, 1, 1000);
            HAL_Delay(2);*/

    // Send SelectNDEFTagApplication command
    M24SR_SelectApplication(M24SR_I2C_READ);

    //TODO try this after select app
    //M24SR_SelectCCfile();

    counter = 2;
    sprintf(msg, "cnt= %d\r\n", counter);
    USART2_PutBuffer((uint8_t *)msg, strlen(msg));
    LL_mDelay(10);
/*
    status = M24SR_SelectSystemfile(M24SR_I2C_READ);
    counter = 3;
        sprintf(msg, "cnt= %d\r\n", counter);
        USART2_PutBuffer((uint8_t *)msg, strlen(msg));
        LL_mDelay(50);

    // Select NDEF file
    M24SR_Verify(M24SR_I2C_ADDR, 0x0001, 0x10, DefaultPassword);*/

    status = M24SR_SelectNDEFfile(M24SR_I2C_READ, 0x0001);

    if (status == M24SR_ERROR_TIMEOUT) {
        sprintf(msg2, "Timeout \n\r");
        //return status;
    } else {
        sprintf(msg2, "status: %d\n\r", status);
    }

    USART2_PutBuffer((uint8_t *)msg2, strlen(msg2));
    LL_mDelay(50);

    // Verify access with password for the NDEF file
    M24SR_Verify(M24SR_I2C_READ , 0x0002, 0x10, DefaultPassword);

    counter = 4;
            sprintf(msg, "cnt= %d\r\n", counter);
            USART2_PutBuffer((uint8_t *)msg, strlen(msg));
            LL_mDelay(50);

    //invalidate
    M24SR_UpdateBinary(M24SR_I2C_READ, 0x0000, 2, zero);

    counter = 5;
            sprintf(msg, "cnt= %d\r\n", counter);
            USART2_PutBuffer((uint8_t *)msg, strlen(msg));
            LL_mDelay(50);

    //write payload
    M24SR_UpdateBinary(M24SR_I2C_READ, 0x0002, total_lenght, ndef_message);
    counter = 6;
    sprintf(msg, "cnt= %d\r\n", counter);
    USART2_PutBuffer((uint8_t *)msg, strlen(msg));
    LL_mDelay(50);

    //commit
    M24SR_UpdateBinary(M24SR_I2C_READ, 0x0000, 2, lenBuf);
    counter = 7;
    sprintf(msg, "cnt= %d\r\n", counter);
    USART2_PutBuffer((uint8_t *)msg, strlen(msg));
    LL_mDelay(50);



    // Verify access with password for the system file
    //M24SR_Verify(NFC_WRITE, 0x0001, 0x10, DefaultPassword);

    // Re-select the NDEF file
    //M24SR_SelectNDEFfile(NFC_WRITE, 0x0001);

    // Read the length of the NDEF file
    uint8_t buffer[2];
    //M24SR_ReadBinary(NFC_WRITE, 0x00, 0x02, buffer);

    // End communication
    M24SR_Deselect(M24SR_I2C_READ);
    M24SR_KillSession(M24SR_I2C_READ);
    counter = 8;
            sprintf(msg, "cnt= %d\r\n", counter);
            USART2_PutBuffer((uint8_t *)msg, strlen(msg));
            LL_mDelay(50);


    return status;
}

/*
uint16_t Read_NDEF_From_NFC_Loose(uint8_t *ndef_buffer,
                                 uint16_t buffer_size,
                                 uint16_t *out_ndef_length)
{
    uint8_t length_bytes[2];
    uint16_t ndef_length = 0;
    uint16_t step = 0;

    *out_ndef_length = 0;

    // Reset any stale session
    M24SR_KillSession(M24SR_I2C_WRITE);
    LL_mDelay(5);
    step = 1;

    // Select NFC application
    M24SR_SelectApplication(M24SR_I2C_WRITE);
    LL_mDelay(5);
    step = 2;

    // Select NDEF file
    M24SR_SelectNDEFfile(M24SR_I2C_WRITE, 0x0001);
    LL_mDelay(5);
    step = 3;

    // Try to read NLEN (ignore status)
    if (M24SR_ReadBinary(M24SR_I2C_WRITE, 0x00, 0x02, length_bytes)
        != M24SR_ACTION_COMPLETED)
    {
        // Do NOT abort — just continue
        LL_mDelay(5);
        step = 4;
    }

    ndef_length =
        ((uint16_t)length_bytes[0] << 8) |
         (uint16_t)length_bytes[1];

    // Clamp length instead of failing
    if (ndef_length == 0 || ndef_length > buffer_size)
    {
        ndef_length = buffer_size;
        step = 5;
    }

    // Try to read payload (ignore status)
    M24SR_ReadBinary(M24SR_I2C_WRITE,
                     0x02,
                     ndef_length,
                     ndef_buffer);

    LL_mDelay(5);

    // End session (always)
    M24SR_Deselect(M24SR_I2C_WRITE);

    *out_ndef_length = ndef_length;
    return step;   // Always report success
}*/

/*
uint16_t arduino_read_NDEF_From_NFC(uint8_t *ndef_buf,
                            uint16_t buf_size,
                            uint16_t *ndef_len)
{
    uint8_t rx[64];
    uint8_t len_hi, len_lo;
    uint16_t length;

    *ndef_len = 0;

    // 1. SELECT I2C (kill RF) — EXACTLY like Arduino
    uint8_t select_i2c = 0x52;
    if (m24sr_write(&select_i2c, 1) != HAL_OK)
        return 1;
    HAL_Delay(2);

    // 2. SELECT NFC APPLICATION
    uint8_t select_app[] = {
        0x02,0x00,0xA4,0x04,0x00,0x07,
        0xD2,0x76,0x00,0x00,0x85,0x01,0x01,
        0x00,0x35,0xC0
    };

    if (m24sr_write(select_app, sizeof(select_app)) != HAL_OK)
        return 2;
    HAL_Delay(2);
    if (m24sr_read(rx, 5) != HAL_OK)
        return 3;

    // 3. SELECT NDEF FILE (0001)
    uint8_t select_ndef[] = {
        0x02,0x00,0xA4,0x00,0x0C,0x02,0x00,0x01,0x3E,0xFD
    };

    if (m24sr_write(select_ndef, sizeof(select_ndef)) != HAL_OK)
        return 4;
    HAL_Delay(2);
    if (m24sr_read(rx, 5) != HAL_OK)
        return 5;

    // 4. READ NLEN (2 bytes at offset 0)
    uint8_t read_len[] = {
        0x03,0x00,0xB0,0x00,0x00,0x02,0x40,0x79
    };

    if (m24sr_write(read_len, sizeof(read_len)) != HAL_OK)
        return 6;
    HAL_Delay(2);
    if (m24sr_read(rx, 7) != HAL_OK)
        return 7;

    len_hi = rx[1];
    len_lo = rx[2];
    length = (len_hi << 8) | len_lo;

    if (length == 0 || length > buf_size)
        return 8;

    // 5. READ NDEF PAYLOAD
    uint8_t read_msg[] = {
        0x02,0x00,0xB0,0x00,0x02,
        (uint8_t)length,   // patched length
        0x00,0x00           // CRC placeholder
    };

    // Compute CRC like Arduino
    ComputeCrc(read_msg, sizeof(read_msg)-2,
               &read_msg[6], &read_msg[7]);

    if (m24sr_write(read_msg, sizeof(read_msg)) != HAL_OK)
        return 9;
    HAL_Delay(2);

    // protocol overhead = PCB + SW1 + SW2 + CRC = 5
    if (m24sr_read(rx, length + 5) != HAL_OK)
        return 10;

    memcpy(ndef_buf, &rx[1], length);
    *ndef_len = length;

    // 6. DESELECT I2C
    uint8_t deselect[] = {0xC2,0xE0,0xB4};
    m24sr_write(deselect, sizeof(deselect));
    HAL_Delay(2);

    return 0; // SUCCESS
}*/
