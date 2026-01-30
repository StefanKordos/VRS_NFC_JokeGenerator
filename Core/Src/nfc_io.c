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
	//status = M24SR_GetSession(M24SR_I2C_READ); //-> helped increasing trials and timeout in isdeviceready
	status = M24SR_KillSession(M24SR_DEVICE_ADDR_W);
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
	uint8_t ndef[256];  // must be >= nlen
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

    //uint8_t langLen = ndef_buffer[4] & 0x3F;
    //USART2_PutBuffer(ndef_buffer[5 + langLen], nlen - 1 - langLen);
    //PrintHex(ndef_buffer, nlen);
    //LL_mDelay(20);

    //M24SR_KillSession(M24SR_I2C_READ);

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


//------------------alternate driver--------------------
/* Private variables */
//static I2C_HandleTypeDef *pI2C = NULL;
static uint8_t gPCB = 0x02;  /* Global PCB tracker */

/* I2C timing */
#define M24SR_I2C_TIMEOUT       200
#define M24SR_RETRY_COUNT       5
#define M24SR_RETRY_DELAY       5

/* Device control bytes */


/* PCB (Protocol Control Byte) */
#define M24SR_PCB_IBLOCK        0x02
#define M24SR_PCB_SBLOCK_DESEL  0xC2

/* NFC T4T Application ID */
static const uint8_t NFC_APP_ID[] = {0xD2, 0x76, 0x00, 0x00, 0x85, 0x01, 0x01};

/* Command buffer */
static uint8_t cmdBuffer[256];
static uint8_t rspBuffer[256];

static uint16_t M24SR_GetStatus(uint8_t *pRsp, uint16_t rspLen)
{
    if (rspLen < 4) return 0xFFFF;
    /* Status is SW1||SW2 before CRC */
    return ((uint16_t)pRsp[rspLen - 4] << 8) | pRsp[rspLen - 3];
}

static void USART2_PrintHex(uint8_t *buf, uint16_t len)
{
    char msg[8];

    for (uint16_t i = 0; i < len; i++) {
        sprintf(msg, "%02X ", buf[i]);
        USART2_PutBuffer((uint8_t *)msg, strlen(msg));
        LL_mDelay(10);
    }

    USART2_PutBuffer((uint8_t *)"\r\n", 2);
    LL_mDelay(10);
}


static uint16_t M24SR_ComputeCRC(uint8_t *data, uint8_t len)
{
    uint16_t crc = 0x6363;

    for (uint8_t i = 0; i < len; i++) {
        uint8_t byte = data[i];
        byte = (byte ^ (uint8_t)(crc & 0x00FF));
        byte = (byte ^ (byte << 4));
        crc = (crc >> 8) ^ ((uint16_t)byte << 8) ^ ((uint16_t)byte << 3) ^ ((uint16_t)byte >> 4);
    }

    return crc;
}



static M24SR_Status M24SR_SendReceive(uint8_t *pCmd, uint8_t cmdLen, uint8_t *pRsp, uint16_t *pRspLen)
{
    HAL_StatusTypeDef status;
    uint16_t crc;
    uint8_t retry;

    /* Append CRC to command */
    crc = M24SR_ComputeCRC(pCmd, cmdLen);
    pCmd[cmdLen] = (uint8_t)(crc & 0xFF);
    pCmd[cmdLen + 1] = (uint8_t)((crc >> 8) & 0xFF);
    cmdLen += 2;

    /* Send command with retry */
    for (retry = 0; retry < M24SR_RETRY_COUNT; retry++) {
        status = HAL_I2C_Master_Transmit(&hi2c1, M24SR_DEVICE_ADDR_W, pCmd, cmdLen, M24SR_I2C_TIMEOUT);
        if (status == HAL_OK) break;
        HAL_Delay(M24SR_RETRY_DELAY);
    }

    if (status != HAL_OK) {
        return M24SR_NACK;
    }

    /* Wait for device to be ready and receive response */
    HAL_Delay(5);

    //is busy test
    /*if (__HAL_I2C_GET_FLAG(&hi2c1, I2C_FLAG_BUSY)) {
        USART2_PutBuffer((uint8_t *)"I2C BUSY stuck\r\n", 16);
        HAL_Delay(10);
    }*/

    for (retry = 0; retry < M24SR_RETRY_COUNT; retry++) {
        status = HAL_I2C_Master_Receive(&hi2c1, M24SR_DEVICE_ADDR_R, pRsp, 256, M24SR_I2C_TIMEOUT);
        if (__HAL_I2C_GET_FLAG(&hi2c1, I2C_FLAG_BUSY)) {
                USART2_PutBuffer((uint8_t *)"I2C BUSY stuck\r\n", 16);
                HAL_Delay(10);
            }
        if (status == HAL_OK) break;
        HAL_Delay(M24SR_RETRY_DELAY);
    }

    if (status != HAL_OK) {
    	char msg[64];
    	sprintf(msg, "rec stat = %d\r\n", status);
    	USART2_PutBuffer((uint8_t *)msg, strlen(msg));
    	HAL_Delay(10);
        return M24SR_TIMEOUT;
    }

    /* Find response length (look for valid response structure) */
    /* Response: PCB + [Data] + SW1 + SW2 + CRC16 */
    /* Minimum valid response is 5 bytes: PCB + SW1 + SW2 + CRC(2) */
    if (pRsp[0] == 0x02 || pRsp[0] == 0x03) {
        /* I-Block response - find length by scanning for valid CRC */
        for (uint16_t len = 5; len < 256; len++) {
            crc = M24SR_ComputeCRC(pRsp, len - 2);
            if ((pRsp[len - 2] == (crc & 0xFF)) && (pRsp[len - 1] == ((crc >> 8) & 0xFF))) {
                *pRspLen = len;
                return M24SR_OK;
            }
        }
    }

    /* Default response length for minimal response */
    *pRspLen = 5;
    return M24SR_OK;
}



M24SR_Status M24SR_KillRFSession(void)
{
    HAL_StatusTypeDef status;
    uint8_t retry;
    uint8_t killCmd[] = {0x52}; /* KILL RF session command */

    for (retry = 0; retry < M24SR_RETRY_COUNT; retry++) {
        status = HAL_I2C_Master_Transmit(&hi2c1, M24SR_DEVICE_ADDR_W, killCmd, 1, M24SR_I2C_TIMEOUT);
        if (status == HAL_OK) break;
        HAL_Delay(M24SR_RETRY_DELAY);
    }

    HAL_Delay(5); /* Wait for session switch */

    /* Reset PCB for new session */
    gPCB = 0x02;

    return (status == HAL_OK) ? M24SR_OK : M24SR_ERROR;
}


M24SR_Status M24SR_SelectApplication_alt(void)
{
    uint16_t rspLen;
    M24SR_Status status;

    /* Build SELECT command for NFC T4T application */
    cmdBuffer[0] = gPCB;   /* PCB */
    gPCB ^= 0x01;          /* Toggle for next command */
    cmdBuffer[1] = 0x00;               /* CLA */
    cmdBuffer[2] = 0xA4;               /* INS: SELECT */
    cmdBuffer[3] = 0x04;               /* P1: Select by name */
    cmdBuffer[4] = 0x00;               /* P2 */
    cmdBuffer[5] = 0x07;               /* Lc: App ID length */
    memcpy(&cmdBuffer[6], NFC_APP_ID, 7);
    cmdBuffer[13] = 0x00;              /* Le */

    status = M24SR_SendReceive(cmdBuffer, 14, rspBuffer, &rspLen);
/*
    char msg[64];
    sprintf(msg, "stat = %d\r\n", status);
    USART2_PutBuffer((uint8_t *)msg, strlen(msg));
    LL_mDelay(20);*/


    if (status != M24SR_OK) return status;

    if (M24SR_GetStatus(rspBuffer, rspLen) != M24SR_STATUS_OK) {
    	USART2_PrintHex(rspBuffer, rspLen);
    	LL_mDelay(100);
        return M24SR_ERROR;
    }

    return M24SR_OK;
}

M24SR_Status M24SR_SelectCCFile_alt(void)
{
    uint16_t rspLen;
    M24SR_Status status;

    /* Build SELECT command for CC file */
    cmdBuffer[0] = gPCB;
    gPCB ^= 0x01;
    cmdBuffer[1] = 0x00;               /* CLA */
    cmdBuffer[2] = 0xA4;               /* INS: SELECT */
    cmdBuffer[3] = 0x00;               /* P1: Select by ID */
    cmdBuffer[4] = 0x0C;               /* P2: First or only occurrence */
    cmdBuffer[5] = 0x02;               /* Lc */
    cmdBuffer[6] = (M24SR_CC_FILE_ID >> 8) & 0xFF;
    cmdBuffer[7] = M24SR_CC_FILE_ID & 0xFF;

    status = M24SR_SendReceive(cmdBuffer, 8, rspBuffer, &rspLen);

    if (status != M24SR_OK) return status;

    if (M24SR_GetStatus(rspBuffer, rspLen) != M24SR_STATUS_OK) {
        return M24SR_ERROR;
    }

    return M24SR_OK;
}

M24SR_Status M24SR_SelectNDEFFile_alt(void)
{
    uint16_t rspLen;
    M24SR_Status status;

    /* Build SELECT command for NDEF file */
    cmdBuffer[0] = gPCB;
    gPCB ^= 0x01;
    cmdBuffer[1] = 0x00;               /* CLA */
    cmdBuffer[2] = 0xA4;               /* INS: SELECT */
    cmdBuffer[3] = 0x00;               /* P1: Select by ID */
    cmdBuffer[4] = 0x0C;               /* P2: First or only occurrence */
    cmdBuffer[5] = 0x02;               /* Lc */
    cmdBuffer[6] = (M24SR_NDEF_FILE_ID >> 8) & 0xFF;
    cmdBuffer[7] = M24SR_NDEF_FILE_ID & 0xFF;

    status = M24SR_SendReceive(cmdBuffer, 8, rspBuffer, &rspLen);

    if (status != M24SR_OK) return status;

    if (M24SR_GetStatus(rspBuffer, rspLen) != M24SR_STATUS_OK) {
        return M24SR_ERROR;
    }

    return M24SR_OK;
}

M24SR_Status M24SR_ReadBinary_alt(uint16_t offset, uint8_t length, uint8_t *pData)
{
    uint16_t rspLen;
    M24SR_Status status;

    /* Build READ BINARY command */
    cmdBuffer[0] = gPCB;
    gPCB ^= 0x01;
    cmdBuffer[1] = 0x00;               /* CLA */
    cmdBuffer[2] = 0xB0;               /* INS: READ BINARY */
    cmdBuffer[3] = (offset >> 8) & 0xFF;  /* P1: Offset MSB */
    cmdBuffer[4] = offset & 0xFF;         /* P2: Offset LSB */
    cmdBuffer[5] = length;                /* Le: Number of bytes to read */

    status = M24SR_SendReceive(cmdBuffer, 6, rspBuffer, &rspLen);

    if (status != M24SR_OK) return status;

    uint16_t sw = M24SR_GetStatus(rspBuffer, rspLen);
    if (sw != M24SR_STATUS_OK && sw != M24SR_STATUS_EOF) {
        return M24SR_ERROR;
    }

    /* Copy data (skip PCB, exclude SW and CRC) */
    uint16_t dataLen = rspLen - 5; /* PCB(1) + SW(2) + CRC(2) */
    if (dataLen > length) dataLen = length;
    memcpy(pData, &rspBuffer[1], dataLen);

    return M24SR_OK;
}


M24SR_Status M24SR_ReadNDEFLength_alt(uint16_t *pLength)
{
    uint8_t lenBuf[2];
    M24SR_Status status;

    /* Read first 2 bytes of NDEF file (length field) */
    status = M24SR_ReadBinary_alt(0, 2, lenBuf);

    if (status != M24SR_OK) return status;

    *pLength = ((uint16_t)lenBuf[0] << 8) | lenBuf[1];

    return M24SR_OK;
}


M24SR_Status M24SR_ReadNDEF_alt(uint8_t *pData, uint16_t maxLen, uint16_t *pActualLen)
{
    M24SR_Status status;
    uint16_t ndefLen;
    uint16_t bytesRead = 0;
    uint16_t offset = 2; /* Skip length field */
    uint8_t chunkSize;

    /* First, get NDEF length */
    status = M24SR_ReadNDEFLength_alt(&ndefLen);
    if (status != M24SR_OK) return status;

    if (ndefLen > maxLen) ndefLen = maxLen;

    /* Read NDEF data in chunks */
    while (bytesRead < ndefLen) {
        chunkSize = ((ndefLen - bytesRead) > 246) ? 246 : (ndefLen - bytesRead);

        status = M24SR_ReadBinary_alt(offset, chunkSize, &pData[bytesRead]);
        if (status != M24SR_OK) return status;

        bytesRead += chunkSize;
        offset += chunkSize;
    }

    *pActualLen = bytesRead;

    return M24SR_OK;
}

M24SR_Status M24SR_Deselect_alt(void)
{
    HAL_StatusTypeDef status;
    uint16_t crc;

    /* S-Block DESELECT command */
    cmdBuffer[0] = M24SR_PCB_SBLOCK_DESEL;

    crc = M24SR_ComputeCRC(cmdBuffer, 1);
    cmdBuffer[1] = (uint8_t)(crc & 0xFF);
    cmdBuffer[2] = (uint8_t)((crc >> 8) & 0xFF);

    status = HAL_I2C_Master_Transmit(&hi2c1, M24SR_DEVICE_ADDR_W, cmdBuffer, 3, M24SR_I2C_TIMEOUT);

    HAL_Delay(5);

    return (status == HAL_OK) ? M24SR_OK : M24SR_ERROR;
}


