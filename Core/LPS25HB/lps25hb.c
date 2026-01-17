/*
 * lps25hb.c
 *
 *  Created on: Nov 1, 2025
 *      Author: stefan
 */

#include "lps25hb.h"

/* Declaration and initialization of callback functions */
static void (* lps25hb_i2c_send_byte)(uint8_t slave_address, uint8_t register_address, uint8_t data) = 0;
static uint8_t (* lps25hb_i2c_read_byte)(uint8_t slave_address, uint8_t register_address) = 0;
static uint16_t (* lps25hb_i2c_read_2bytes)(uint8_t slave_address, uint8_t register_address) = 0;
static uint32_t (* lps25hb_i2c_read_3bytes)(uint8_t slave_address, uint8_t register_address) = 0;

//register callback functions
void lps25hb_i2c_send_byte_RegisterCallback(void *callback){
	if(callback != 0){
		lps25hb_i2c_send_byte = callback;
	}
}

void lps25hb_i2c_read_byte_RegisterCallback(void *callback){
	if(callback != 0){
		lps25hb_i2c_read_byte = callback;
	}
}

void lps25hb_i2c_read_2bytes_RegisterCallback(void *callback){
	if(callback != 0){
		lps25hb_i2c_read_2bytes = callback;
	}
}

void lps25hb_i2c_read_3bytes_RegisterCallback(void *callback){
	if(callback != 0){
		lps25hb_i2c_read_3bytes = callback;
	}
}


//sensor init
void lps25hb_init(){
	if(lps25hb_i2c_read_byte(LPS25HB_ADDRESS, LPS25HB_WHO_AM_I_ADDRESS) == LPS25HB_WHO_AM_I_VALUE){
		//write init into CTRL_REG1
		lps25hb_i2c_send_byte(LPS25HB_ADDRESS, CTRL_REG1, LPS25HB_INIT_VALUE);
	}
}


uint32_t lps25hb_read_pressure(){
	uint32_t sensor_pressure = lps25hb_i2c_read_3bytes(LPS25HB_ADDRESS, PRESS_OUT_XL);
	return sensor_pressure;
}

float pressure_calculation(uint32_t sensor_pressure) {
	float float_sensor_pressure = (float)sensor_pressure;
	float pressure = float_sensor_pressure / (float)4096;
	return pressure;
}

float height_calculation(float pressure, float p_ground, float h_ground, float ro, float g) {
	//float height = -1*((pressure-p_ground)-(ro*g))+h_ground; //maybe not working -> probably wrong, mixing dimension

	float height = h_ground - ((pressure - p_ground) * 100.0) / (ro * g); //works
	return height;
}

