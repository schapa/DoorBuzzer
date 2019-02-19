/*
 * bsp.c
 *
 *  Created on: May 18, 2016
 *      Author: shapa
 */

#include "bsp.h"
#include "system.h"
#include "systemTimer.h"
#include "Queue.h"
#include "timers.h"

#include "stm32f0xx_rcc.h"
#include "stm32f0xx_gpio.h"
#include "stm32f0xx_adc.h"
#include "stm32f0xx_iwdg.h"
#include "stm32f0xx_tim.h"

#include <stddef.h>
#include <stdlib.h>

#define ADC_TIMEOUT (BSP_TICKS_PER_SECOND/50)

static inline void initialize_RCC(void);
static inline void initWdt(void);
static inline void initPWM_TIM(void);
static inline void initPWM_OC(void);

static void setSystemLed(_Bool state);

_Bool BSP_Init(void) {
	initialize_RCC();
	initWdt();
	BSP_InitGpio();
	System_init(setSystemLed);

	initPWM_TIM();
	initPWM_OC();
	BSP_SetPinPWM(0);

	return true;
}

void BSP_FeedWatchdog(void) {
	IWDG_ReloadCounter();
}

void BSP_SetPinPWM(const uint32_t value) {
	TIM3->CCR2 = value;
}

static void initialize_RCC(void) {

	RCC_HSEConfig(RCC_HSE_OFF);
	RCC_WaitForHSEStartUp(); // really we wait for shutdown
	RCC_ADCCLKConfig(RCC_ADCCLK_PCLK_Div4);

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOF, ENABLE);

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

	GPIO_DeInit(GPIOA);
	GPIO_DeInit(GPIOB);
	GPIO_DeInit(GPIOF);
}

static void initWdt(void) {
//	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
//	IWDG_SetPrescaler(IWDG_Prescaler_32);
//	IWDG_SetReload(0x0FFF);
//	IWDG_ReloadCounter();
//	IWDG_Enable();
}

static void initPWM_TIM(void) {
	TIM_TimeBaseInitTypeDef iface = {
			0x2F,
			TIM_CounterMode_Up,
			0x181,
			TIM_CKD_DIV1,
			0
	};

	TIM_TimeBaseInit(TIM3, &iface);
	TIM_Cmd(TIM3, ENABLE);
}

static void initPWM_OC(void) {

    TIM_OCInitTypeDef pwm = {
		TIM_OCMode_PWM1,
		TIM_OutputState_Enable,
		0,
		0,
		TIM_OCPolarity_High,
		0, 0, 0
    };
    TIM_OC2Init(TIM3, &pwm);
    TIM_CCxCmd(TIM3, TIM_Channel_2, TIM_CCx_Enable);
}


static inline void setSystemLed(_Bool state) {
	BSP_SetPinVal(BSP_Pin_LED, state);
}
