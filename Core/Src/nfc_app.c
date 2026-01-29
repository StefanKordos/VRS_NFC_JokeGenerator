/*
 * nfc_app.c
 *
 *  Created on: Jan 20, 2026
 *      Author: radoj
 ******************************************************************************
 * - Parses NFC text requests
 * - Decides requested operation
 * - Prepares response
 * - Writes response as NDEF to NFC tag
 *
 * Depends on:
 *  - Section 2: receiving NFC text
 *  - SD card module: joke storage
 */

#include "nfc_io.h"
#include "nfc_app.h"

//#include "sdCard.h" TODO
//#include "m24sr.h"
#include "../M24SR/m24sr.h"
#include <string.h>

NFC_Request_t NFC_ParseRequest(char *rxText)
{
    if (rxText == NULL)
        return NFC_REQ_INVALID;

    if (strncmp(rxText, "GET_JOKE", 8) == 0)
        return NFC_REQ_GET_JOKE;

    if (strncmp(rxText, "ADD_JOKE:", 9) == 0)
        return NFC_REQ_ADD_JOKE;

    return NFC_REQ_INVALID;
}


void NFC_ProcessRequest(char *rxText)
{
    NFC_Request_t request;
    char responseText[253]; //joke text or response with \0
    uint8_t ndefBuffer[300]; //octets
    uint16_t ndefLength;

    request = NFC_ParseRequest(rxText);

    switch (request)
    {
        case NFC_REQ_GET_JOKE:
        	/*
			 * TODO (section SD):
			 * - read random joke from SD card
			 * - store into responseText
			 */

        	//joke text = 255 - (status byte + language code) SR -
        	//PAYLOAD fields of size ranging between 0 to 255 octets. NFCForum-TS-NDEF_1.0 s.3.2.4
        	//TEST
        	const char *testJoke = "This is a static test joke";
        	if (strlen(testJoke) > 252)
        	{
				printf("Joke is too long");
				return;
        	}
        	strcpy(responseText, testJoke);
        	break;

        case NFC_REQ_ADD_JOKE:
        	/*
			 * TODO (section SD):
			 * - extract joke text from rxText
			 * - store joke into SD card
			 */
			strcpy(responseText, "ADD_JOKE request received");
			/*if joke successfully added
			 * return joke added to database
			 */
			break;

        default:
            /*
             * Not valid request
             * Write error message to NFC
             */
        	strcpy(responseText, "INVALID REQUEST");
		    break;
    }
    /*
     * Convert response text to NDEF and write to NFC tag via I2C
     */
    ndefLength = Convert_to_NDEF(responseText, ndefBuffer);
    if (ndefLength > 0){
    	Write_Joke_to_NFC(ndefBuffer, ndefLength);
    } else {
        printf("Error: NDEF Conversion failed.\r\n");
    }
}
