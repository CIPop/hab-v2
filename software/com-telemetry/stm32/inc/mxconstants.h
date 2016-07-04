#pragma once

#include "stm32l073xx.h"
#include "stm32l0xx_hal_gpio.h"

#define COPERNICUS_GPIO               GPIOB
#define COPERNICUS_UART               LPUART1
#define COPERNICUS_UART_TX_Pin        GPIO_PIN_10
#define COPERNICUS_UART_RX_Pin        GPIO_PIN_11
#define COPERNICUS_UART_IRQn          LPUART1_IRQn
#define COPERNICUS_GPIO_AF4_UART      GPIO_AF4_LPUART1
#define COPERNICUS_LPUART_IRQHandler  LPUART1_IRQHandler
#define COPERNICUS_GPIO_CLK_ENABLE    __HAL_RCC_GPIOB_CLK_ENABLE
#define COPERNICUS_UART_CLK_ENABLE    __HAL_RCC_LPUART1_CLK_ENABLE
#define COPERNICUS_UART_CLK_DISABLE   __HAL_RCC_LPUART1_CLK_DISABLE
#define COPERNICUS_UART_FORCE_RESET   __HAL_RCC_LPUART1_FORCE_RESET
#define COPERNICUS_UART_RELEASE_RESET __HAL_RCC_LPUART1_RELEASE_RESET

#define TRACE_UART                      USART2
#define TRACE_UART_IRQHandler           USART2_IRQHandler
#define TRACE_DMA_RX_IRQHandler         DMA1_Channel4_5_6_7_IRQHandler
#define TRACE_DMA_CLK_ENABLE()          __HAL_RCC_DMA1_CLK_ENABLE()
#define TRACE_UART_CLK_ENABLE()         __HAL_RCC_USART2_CLK_ENABLE()
#define TRACE_UART_RX_GPIO_CLK_ENABLE() __HAL_RCC_GPIOA_CLK_ENABLE()
#define TRACE_UART_TX_GPIO_CLK_ENABLE() __HAL_RCC_GPIOA_CLK_ENABLE()
#define TRACE_UART_TX_PIN               GPIO_PIN_2
#define TRACE_UART_TX_AF                GPIO_AF4_USART2
#define TRACE_UART_TX_GPIO_PORT         GPIOA
#define TRACE_UART_RX_PIN               GPIO_PIN_3
#define TRACE_UART_RX_AF                GPIO_AF4_USART2
#define TRACE_UART_RX_GPIO_PORT         GPIOA
#define TRACE_UART_TX_DMA_CHANNEL       DMA1_Channel7
#define TRACE_UART_RX_DMA_CHANNEL       DMA1_Channel6
#define TRACE_UART_TX_DMA_REQUEST       DMA_REQUEST_4
#define TRACE_UART_RX_DMA_REQUEST       DMA_REQUEST_4
#define TRACE_UART_DMA_TX_IRQn          DMA1_Channel4_5_6_7_IRQn
#define TRACE_UART_DMA_RX_IRQn          DMA1_Channel4_5_6_7_IRQn
#define TRACE_UART_IRQn                 USART2_IRQn
#define TRACE_UART_FORCE_RESET()        __HAL_RCC_USART2_FORCE_RESET()
#define TRACE_UART_RELEASE_RESET()      __HAL_RCC_USART2_RELEASE_RESET()

#define HX1_DAC                 DAC1
#define HX1_DAC_CHANNEL         DAC_CHANNEL_1
#define HX1_DAC_TRIGGER         DAC_TRIGGER_T6_TRGO
#define HX1_DAC_ALIGN           DAC_ALIGN_12B_R
#define HX1_DAC_CLK_ENABLE      __HAL_RCC_DAC_CLK_ENABLE
#define HX1_DAC_FORCE_RESET     __HAL_RCC_DAC_FORCE_RESET
#define HX1_DAC_RELEASE_RESET   __HAL_RCC_DAC_RELEASE_RESET
#define HX1_TIMER               TIM6
#define HX1_TIMER_CLK_ENABLE    __HAL_RCC_TIM6_CLK_ENABLE
#define HX1_TIMER_FORCE_RESET   __HAL_RCC_TIM6_FORCE_RESET
#define HX1_TIMER_RELEASE_RESET __HAL_RCC_TIM6_RELEASE_RESET
#define HX1_DMA_INSTANCE        DMA1_Channel2
#define HX1_DMA_IRQn            DMA1_Channel2_3_IRQn
#define HX1_DMA_IRQHandler      DMA1_Channel2_3_IRQHandler
#define HX1_DMA_CLK_ENABLE      __HAL_RCC_DMA1_CLK_ENABLE
#define HX1_GPIO_CLK_ENABLE     __HAL_RCC_GPIOA_CLK_ENABLE
#define HX1_GPIO_PIN            GPIO_PIN_4
#define HX1_GPIO_PORT           GPIOA

#define HX1_ENABLE_Pin       GPIO_PIN_5
#define HX1_ENABLE_GPIO_Port GPIOA
