/* Copyright 2012 Adam Green (http://mbed.org/users/AdamGreen/)

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as published
   by the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
/* Routines used by mri that are specific to the STM32F429ZI device. */
#ifndef _STM32F429XX_H_
#define _STM32F429XX_H_

#include <stdint.h>
#include <token.h>
#include "stm32f429xx_usart.h"

/* Flags that can be set in Stm32f429xxState::flags */
#define STM32F429XX_UART_FLAGS_SHARE        1
#define STM32F429XX_UART_FLAGS_MANUAL_BAUD  2

/* Flag to indicate whether context will contain FPU registers or not. */
#define MRI_DEVICE_HAS_FPU 1

typedef struct {
    const UartConfiguration*  pCurrentUart;
    uint32_t                  flags;
} Stm32f429xxState;

extern Stm32f429xxState __mriStm32f429xxState;

void __mriStm32f429xx_Init(Token* pParameterTokens);

#endif /* _STM32F429XX_H_ */
