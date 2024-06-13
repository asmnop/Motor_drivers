/*
 * TB6612FNG.h
 *
 *  Created on: Jun 13, 2024
 *      Author: asmnop
 */

#ifndef INC_TB6612FNG_H_
#define INC_TB6612FNG_H_


#include "stdint.h"
#include "gpio.h"


typedef struct TB6612FNG_LINK_STRUCT
{
	GPIO_TypeDef *port_IN1;
	GPIO_TypeDef *port_IN2;
	GPIO_TypeDef *port_PWM;
	uint16_t pin_IN1;
	uint16_t pin_IN2;
	uint16_t pin_PWM;
	TIM_HandleTypeDef *htim;
	uint32_t Channel;
	uint16_t duty;
}TB6612FNG_t;


extern TB6612FNG_t MOTOR_1;

void TM6612FNG_init(TB6612FNG_t *motor);
void TB6612FNG_dir(TB6612FNG_t *motor, const uint8_t dir);
void TB6612FNG_speed(TB6612FNG_t *motor, uint8_t speed);


#endif /* INC_TB6612FNG_H_ */












