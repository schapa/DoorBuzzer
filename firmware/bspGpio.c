/*
 * bspGpio.c
 *
 *  Created on: Jan 19, 2017
 *      Author: shapa
 */

#include <stdlib.h>

#include "bsp.h"
#include "stm32f0xx_gpio.h"
#include "stm32f0xx_exti.h"
#include "Queue.h"

typedef struct {
	GPIO_TypeDef *const port;
	const GPIO_InitTypeDef setting;
} BspGpioConfig_t;

static const BspGpioConfig_t s_gpioConfig[] = {

	[BSP_Pin_PWM] = { GPIOA, { GPIO_Pin_7, GPIO_Mode_AF,
		GPIO_Speed_Level_3, GPIO_OType_PP,  GPIO_PuPd_NOPULL} },

	[BSP_Pin_LED] = { GPIOA, { GPIO_Pin_5, GPIO_Mode_OUT,
		GPIO_Speed_Level_1, GPIO_OType_PP,  GPIO_PuPd_NOPULL} },

	[BSP_Pin_BUTTON] = { GPIOA, { GPIO_Pin_4, GPIO_Mode_IN,
		GPIO_Speed_Level_1, GPIO_OType_PP,  GPIO_PuPd_UP} },
};

void BSP_InitGpio(void) {
	static const size_t size = sizeof(s_gpioConfig)/sizeof(*s_gpioConfig);
	for (size_t i = 0; i < size; i++)
		GPIO_Init((GPIO_TypeDef*)s_gpioConfig[i].port, (GPIO_InitTypeDef*)&s_gpioConfig[i].setting);

	GPIO_PinAFConfig((GPIO_TypeDef*)s_gpioConfig[BSP_Pin_PWM].port, GPIO_PinSource7, GPIO_AF_1);

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
	EXTI_InitTypeDef initStructure;
	initStructure.EXTI_Line = EXTI_Line4;
	initStructure.EXTI_LineCmd = ENABLE;
	initStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	initStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_Init(&initStructure);
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource4);

	NVIC_InitTypeDef irq = {
			EXTI4_15_IRQn,
			0,
			ENABLE
	};
	NVIC_Init(&irq);
}

void BSP_SetPinVal(const BSP_Pin_t pin, const _Bool state) {
	if (pin > BSP_Pin_Last)
		return;
	if (state)
		s_gpioConfig[pin].port->BSRR = s_gpioConfig[pin].setting.GPIO_Pin;
	else
		s_gpioConfig[pin].port->BRR = s_gpioConfig[pin].setting.GPIO_Pin;
}

_Bool BSP_GetPinVal(const BSP_Pin_t pin) {
	if (pin > BSP_Pin_Last)
		return false;
	return GPIO_ReadInputDataBit((GPIO_TypeDef*)s_gpioConfig[pin].port, s_gpioConfig[pin].setting.GPIO_Pin);
}

void EXTI4_15_IRQHandler(void) {
	if(EXTI_GetITStatus(EXTI_Line4)) {
		EXTI_ClearITPendingBit(EXTI_Line4);
		EventQueue_Push(EVENT_STOP, NULL, NULL);
	}
}
