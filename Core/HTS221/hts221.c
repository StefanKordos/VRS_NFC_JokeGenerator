/*
 * hts221.c
 *
 *  Created on: Nov 1, 2025
 *      Author: stefan
 */

#include "hts221.h"

static void (* hts221_i2c_send_byte)(uint8_t slave_address, uint8_t register_address, uint8_t data) = 0;
static uint8_t (* hts221_i2c_read_byte)(uint8_t slave_address, uint8_t register_address) = 0;
static uint16_t (* hts221_i2c_read_2bytes)(uint8_t slave_address, uint8_t register_address) = 0;

//callback registration

void hts221_i2c_send_byte_RegisterCallback(void *callback){
	if(callback != 0){
		hts221_i2c_send_byte = callback;
	}
}

void hts221_i2c_read_byte_RegisterCallback(void *callback){
	if(callback != 0){
		hts221_i2c_read_byte = callback;
	}
}

void hts221_i2c_read_2bytes_RegisterCallback(void *callback){
	if(callback != 0){
		hts221_i2c_read_2bytes = callback;
	}
}


//init the sensor
void hts221_init(){
	if(hts221_i2c_read_byte(HTS221_ADDRESS, HTS221_WHO_AM_I_ADDRESS) == HTS221_WHO_AM_I_VALUE){
		//write init into CTRL_REG1
		hts221_i2c_send_byte(HTS221_ADDRESS, CTRL_REG1, HTS221_INIT_VALUE);
	}
}


//temp reading
uint16_t hts221_read_temp() {
	uint16_t sensor_temp = hts221_i2c_read_2bytes(HTS221_ADDRESS, TEMP_OUT_L);
	return sensor_temp;

}


//calibration registers reading
uint8_t read_T0_degC_x8() {
	uint8_t T0_degC_x8 = hts221_i2c_read_byte(HTS221_ADDRESS, T0_DEGC_X8);
	return T0_degC_x8;
}

uint8_t read_T1_degC_x8() {
	uint8_t T1_degC_x8 = hts221_i2c_read_byte(HTS221_ADDRESS, T1_DEGC_X8);
	return T1_degC_x8;
}

uint8_t read_T1_T0_msb() {
	uint8_t T1_T0_msb = hts221_i2c_read_byte(HTS221_ADDRESS, T1_T0_MSB);
	return T1_T0_msb;
}

uint16_t read_T0_OUT() {
	uint16_t T0_out = hts221_i2c_read_2bytes(HTS221_ADDRESS, T0_OUT);
	return T0_out;
}

uint16_t read_T1_OUT() {
	uint16_t T1_out = hts221_i2c_read_2bytes(HTS221_ADDRESS, T1_OUT);
	return T1_out;
}

uint16_t T0_degC_calculation(uint8_t T1_T0_msb, uint8_t T0_degC_x8) {
	uint16_t  T0_degC_x8_u16 = (((uint16_t)(T1_T0_msb & 0x03)) << 8) | ((uint16_t)T0_degC_x8);
	uint16_t T0_degC = T0_degC_x8_u16>>3;
	return T0_degC;
}

uint16_t T1_degC_calculation(uint8_t T1_T0_msb, uint8_t T1_degC_x8) {
	uint16_t  T1_degC_x8_u16 = (((uint16_t)(T1_T0_msb & 0x0C)) << 6) | ((uint16_t)T1_degC_x8);
	uint16_t T1_degC = T1_degC_x8_u16>>3;
	return T1_degC;
}

//linear interpolation
/*
float degC_calculation(uint16_t sensor_temp, uint16_t T0_out, uint16_t T1_out, uint16_t T0_degC, uint16_t T1_degC) {
	float tmp = ((int)(sensor_temp + 4)) * ((int)(T1_degC - T0_degC));
	float deg_c = (tmp /(T1_out + 4)) + T0_degC;
	return deg_c;
}*/

float degC_calculation(uint16_t sensor_temp, uint16_t T0_out, uint16_t T1_out, uint16_t T0_degC, uint16_t T1_degC) {
	float deg_c = T0_degC +
	    ( (float)(sensor_temp - T0_out) * (T1_degC - T0_degC) )
	    / (float)(T1_out - T0_out);
	return deg_c;
}



// relative humidity

uint16_t hts221_read_humidity() {
	uint16_t sensor_humidity = hts221_i2c_read_2bytes(HTS221_ADDRESS, H_OUT);
	return sensor_humidity;
}

//calib regs
uint8_t read_H0_rH_x2() {
	uint8_t H0_rH_x2 = hts221_i2c_read_byte(HTS221_ADDRESS, H0_RH_X2);
	uint8_t H0_rh = H0_rH_x2>>1;
	return H0_rh;
}

uint8_t read_H1_rH_x2() {
	uint8_t H1_rH_x2 = hts221_i2c_read_byte(HTS221_ADDRESS, H1_RH_X2);
	uint8_t H1_rh = H1_rH_x2>>1;
	return H1_rh;
}

uint16_t read_h0_h0_out() {
	uint16_t h0_h0_out = hts221_i2c_read_2bytes(HTS221_ADDRESS, H0_T0_OUT);
	return h0_h0_out;
}

uint16_t read_h1_h0_out() {
	uint16_t h1_h0_out = hts221_i2c_read_2bytes(HTS221_ADDRESS, H1_T0_OUT);
	return h1_h0_out;
}

uint16_t rh_calculation(uint16_t sensor_humidity, uint8_t H0_rh, uint8_t H1_rh, uint16_t h0_h0_out, uint16_t h1_h0_out) {
	uint32_t tmp = ((int32_t)(sensor_humidity - h0_h0_out)) * ((int32_t)(H1_rh - H0_rh));
	uint16_t rh = (uint16_t)(tmp/(h1_h0_out - h0_h0_out) + H0_rh);
	return rh;
}
