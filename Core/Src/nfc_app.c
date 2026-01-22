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

//#include "sdCard.h" TODO
#include "m24sr.h"
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
    char responseText[256];
    uint8_t ndefBuffer[300];
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
			strcpy(responseText, "GET_JOKE request received");
			break;

        case NFC_REQ_ADD_JOKE:
        	/*
			 * TODO (section SD):
			 * - extract joke text from rxText
			 * - store joke into SD card
			 */
			strcpy(responseText, "ADD_JOKE request received");
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
     * +13 bytes = NDEF Text Record overhead (header + type + language code)
     */
	convert_to_NDEF(responseText, ndefBuffer);
	ndefLength = strlen(responseText) + 13;

	Write_Joke_TO_NFC(ndefBuffer, ndefLength);
}
