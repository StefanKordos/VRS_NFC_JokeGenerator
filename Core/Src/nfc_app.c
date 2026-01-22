/*
 * nfc_app.c
 *
 *  Created on: Jan 20, 2026
 *      Author: radoj
 */

#include "nfc_io.h"

//#include "sdCard.h" TODO
#include "m24sr.h"
#include <string.h>

NFC_Request_t NFC_ParseRequest(char *rxText) //TODO format ?
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
             * 1. Call function from sdCard.c (random joke)
             * 2. result store to responseText
             * 3. convert responseText → NDEF
             * 4. store NDEF do NFC tag
             */
            break;

        case NFC_REQ_ADD_JOKE:
            /*
             * 1. Extract joke from rxText
             * 2. Call function that stores joke to SD
             * 3. Prepare confirmation (success/not success) to responseText
             * 4. Write confirmation to NFC
             */
            break;

        default:
            /*
             * Not valid request
             * Write error message to NFC
             */
            break;
    }
}
