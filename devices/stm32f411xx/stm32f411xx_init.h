/* Copyright 2012 Adam Green     (https://github.com/adamgreen/)
   Copyright 2015 Chang,Jia-Rung (https://github.com/JaredCJR)

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/
/* Routines used by mri that are specific to the STM32F411xx device. */
#ifndef STM32F411XX_H_
#define STM32F411XX_H_

#include <stdint.h>
#include <core/token.h>
#include "stm32f411xx_usart.h"

/* Flags that can be set in Stm32f411xxState::flags */
#define STM32F411XX_UART_FLAGS_SHARE        1
#define STM32F411XX_UART_FLAGS_MANUAL_BAUD  2

/* Flag to indicate whether context will contain FPU registers or not. */
#define MRI_DEVICE_HAS_FPU 1

typedef struct
{
    const UartConfiguration*  pCurrentUart;
    uint32_t                  flags;
} Stm32f411xxState;

extern Stm32f411xxState mriStm32f411xxState;

void mriStm32f411xx_Init(Token* pParameterTokens);

#endif /* STM32F411XX_H_ */
