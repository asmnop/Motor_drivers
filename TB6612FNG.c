/*
 * TB6612FNG.c
 *
 *  Created on: Jun 13, 2024
 *      Author: asmnop
 */


#include "TB6612FNG.h"
#include "tim.h"


void TB6612FNG_test(void)
{
	HAL_GPIO_WritePin(AIN1_GPIO_Port, AIN1_Pin, GPIO_PIN_SET);
	__HAL_TIM_SetCompare(&htim2, TIM_CHANNEL_1, 40);
}

void TB6612FNG_test_2(void)
{
	HAL_GPIO_WritePin(AIN1_GPIO_Port, AIN1_Pin, GPIO_PIN_SET);
	//__HAL_TIM_SetCompare(&htim2, TIM_CHANNEL_1, 70);
	htim2.Instance->CCR1 = 90;
}

void TB6612FNG_init(void)
{
	//	konfiguracja wstępna sterownika,

	HAL_GPIO_WritePin(AIN1_GPIO_Port, AIN1_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(AIN2_GPIO_Port, AIN2_Pin, GPIO_PIN_RESET);

	__HAL_TIM_SetCompare(&htim2, TIM_CHANNEL_1, 0);					//	Równoważne z 'htim2.Instance->CCR1 = 0;'

	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
}

void TB6612FNG_speed(uint8_t speed)
{
	//	-ustawianie prędkości obrotowej wału silnika,
	//	-w rzeczywistości jest to ustawienie wypełnienia sygnału PWM,
	//	-należy pamiętać aby nie przekroczyć wartości z rejestru ARR, który oznacza liczbę, do której zlicza
	//	timer a po jej osiągnięciu jego wartość przyjmuje wartość 0,
	//	wartość speed jest podstawiana do rejestru CCRx = TIM capture/compare register x,

	if(speed >= htim2.Instance->ARR)
		speed = htim2.Instance->ARR;

	__HAL_TIM_SetCompare(&htim2, TIM_CHANNEL_1, speed);
}

void TB6612FNG_dir(const uint8_t dir)
{
	if(dir == 1)
	{
		HAL_GPIO_WritePin(AIN1_GPIO_Port, AIN1_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(AIN2_GPIO_Port, AIN2_Pin, GPIO_PIN_RESET);
	}
	else if(dir == 0)
	{
		HAL_GPIO_WritePin(AIN1_GPIO_Port, AIN1_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(AIN2_GPIO_Port, AIN2_Pin, GPIO_PIN_SET);
	}
}





