#pragma once

#include "global.h"


#define STM32_AFIO_RESETLINE 0
#define STM32_GPIO_RESETLINE(x) ((x) + 2)
#define STM32_GPIOA_RESETLINE STM32_GPIO_RESETLINE(0)
#define STM32_GPIOB_RESETLINE STM32_GPIO_RESETLINE(1)
#define STM32_GPIOC_RESETLINE STM32_GPIO_RESETLINE(2)
#define STM32_GPIOD_RESETLINE STM32_GPIO_RESETLINE(3)
#define STM32_GPIOE_RESETLINE STM32_GPIO_RESETLINE(4)
#define STM32_ADC1_RESETLINE 9
#define STM32_ADC2_RESETLINE 10
#define STM32_TIM1_RESETLINE 11
#define STM32_SPI1_RESETLINE 12
#define STM32_TIM2_RESETLINE 32
#define STM32_TIM3_RESETLINE 33
#define STM32_TIM4_RESETLINE 34
#define STM32_TIM5_RESETLINE 35
#define STM32_TIM6_RESETLINE 36
#define STM32_TIM7_RESETLINE 37
#define STM32_WWDG_RESETLINE 43
#define STM32_SPI2_RESETLINE 46
#define STM32_SPI3_RESETLINE 47
#define STM32_USART2_RESETLINE 49
#define STM32_USART3_RESETLINE 50
#define STM32_UART4_RESETLINE 51
#define STM32_UART5_RESETLINE 52
#define STM32_I2C1_RESETLINE 53
#define STM32_I2C2_RESETLINE 54
#define STM32_CAN1_RESETLINE 57
#define STM32_CAN2_RESETLINE 58
#define STM32_BKP_RESETLINE 59
#define STM32_PWR_RESETLINE 60
#define STM32_DAC_RESETLINE 61
#define STM32_OTGFS_RESETLINE 236
#define STM32_ETHMAC_RESETLINE 238
