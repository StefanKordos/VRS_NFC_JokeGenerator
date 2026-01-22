/*
 * nfc_app.h
 *
 *  Created on: Jan 20, 2026
 *      Author: radoj
 */

#ifndef INC_NFC_APP_H_
#define INC_NFC_APP_H_

/*Types of NFC request*/
typedef enum {
    NFC_REQ_INVALID = 0,
    NFC_REQ_GET_JOKE,
    NFC_REQ_ADD_JOKE
} NFC_Request_t;

/**
 * @brief Analyze received message
 * @param rxText Text received from NFC tag
 * @return type of NFC request
 */
NFC_Request_t NFC_ParseRequest(char *rxText);

/**
 * @brief Process NFC request and write response to NFC tag
 * @param rxText Text received from NFC tag
 */
void NFC_ProcessRequest(char *rxText);

#endif /* INC_NFC_APP_H_ */
