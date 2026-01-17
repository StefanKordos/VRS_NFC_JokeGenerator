/*
 * hts221.h
 *
 *  Created on: Nov 1, 2025
 *      Author: stefan
 */

#ifndef HTS221_H_
#define HTS221_H_

#include "stdint.h" //needed for the uint_t

#define 	HTS221_ADDRESS		        0xBEU
#define 	HTS221_WHO_AM_I_ADDRESS		0x0F
#define 	HTS221_WHO_AM_I_VALUE		0xBCU
#define 	HTS221_INIT_VALUE		    133

#define     CTRL_REG1                   0x20
#define     TEMP_OUT_L                  0x2A
//calibration registers
#define     T0_DEGC_X8                  0x32
#define     T1_DEGC_X8                  0x33
#define     T1_T0_MSB                   0x35
#define     T0_OUT                      0x3C
#define     T1_OUT                      0x3E

//humidity macros
#define     H_OUT                       0x28

//calibration
#define     H0_RH_X2                    0x30
#define     H1_RH_X2                    0x31
#define     H0_T0_OUT                   0x36
#define     H1_T0_OUT                   0x3A


//register callbacks
void hts221_i2c_send_byte_RegisterCallback(void *callback);
void hts221_i2c_read_byte_RegisterCallback(void *callback);
void hts221_i2c_read_2bytes_RegisterCallback(void *callback);

//temperature function declarations
uint16_t hts221_read_temp();
uint16_t hts221_read_humidity();

uint8_t read_T0_degC_x8();
uint8_t read_T1_degC_x8();
uint8_t read_T1_T0_msb();
uint16_t read_T0_OUT();
uint16_t read_T1_OUT();
uint16_t T0_degC_calculation(uint8_t T1_T0_msb, uint8_t T0_degC_x8);
uint16_t T1_degC_calculation(uint8_t T1_T0_msb, uint8_t T1_degC_x8);
float degC_calculation(uint16_t sensor_temp, uint16_t T0_out, uint16_t T1_out, uint16_t T0_degC, uint16_t T1_degC);

//humidity function declarations

uint8_t read_H0_rH_x2();
uint8_t read_H1_rH_x2();
uint16_t read_h0_h0_out();
uint16_t read_h1_h0_out();
uint16_t rh_calculation(uint16_t sensor_humidity, uint8_t H0_rh, uint8_t H1_rh, uint16_t h0_h0_out, uint16_t h1_h0_out);

#endif /* HTS221_H_ */
