/* Copyright 2012 Adam Green     (http://mbed.org/users/AdamGreen/)
   Copyright 2015 Chang,Jia-Rung (https://github.com/JaredCJR)

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
/* Routines used to provide STM32F429xx USART functionality to the mri debugger. */
#ifndef _STM32F429XX_USART_H_
#define _STM32F429XX_USART_H_

#include <stdint.h>
#include <stm32f4xx.h>
#include <token.h>

typedef struct 
{
    USART_TypeDef*     pUartRegisters;
    uint32_t    txFunction;
    uint32_t    rxFunction;
} UartConfiguration;


void __mriStm32f429xxUart_Init(Token* pParameterTokens);

#endif /* _STM32F429XX_USART_H_ */
