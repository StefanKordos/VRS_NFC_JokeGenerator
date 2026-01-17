/*
 * lps25hb.h
 *
 *  Created on: Nov 1, 2025
 *      Author: stefan
 */

#ifndef LPS25HB_H_
#define LPS25HB_H_

#include "stdint.h" //needed for the uint_t

#define 	LPS25HB_ADDRESS		        0xBAU
#define 	LPS25HB_WHO_AM_I_ADDRESS	0x0F
#define 	LPS25HB_WHO_AM_I_VALUE		0xBDU
#define 	LPS25HB_INIT_VALUE		    148

//registers
#define     CTRL_REG1                   0x20
#define     PRESS_OUT_XL                0x28




//register callbacks
void lps25hb_i2c_send_byte_RegisterCallback(void *callback);
void lps25hb_i2c_read_byte_RegisterCallback(void *callback);
void lps25hb_i2c_read_2bytes_RegisterCallback(void *callback);
void lps25hb_i2c_read_3bytes_RegisterCallback(void *callback);

//function declarations
uint32_t lps25hb_read_pressure();
float pressure_calculation(uint32_t sensor_pressure);
float height_calculation(float pressure, float p_ground, float h_ground, float ro, float g);


#endif /* LPS25HB_H_ */
