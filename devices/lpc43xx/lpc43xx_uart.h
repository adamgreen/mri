/* Copyright 2015 Adam Green (http://mbed.org/users/AdamGreen/)

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
/* Routines used to provide LPC176x UART functionality to the mri debugger. */
#ifndef _LPC43XX_UART_H_
#define _LPC43XX_UART_H_

#include <stdint.h>
#include <LPC43xx.h>
#include <token.h>

#define SCU_PIN(GROUP, NUM) (((GROUP) << 16) | (NUM))

typedef struct
{
    LPC_USART_T*        pUartRegisters;
    CGU_BASE_CLK_T      baseClock;
    CCU_CLK_T           registerClock;
    CCU_CLK_T           peripheralClock;
    uint32_t            txPin;
    uint32_t            txFunction;
    uint32_t            rxPin;
    uint32_t            rxFunction;
} UartConfiguration;

void __mriLpc43xxUart_Init(Token* pParameterTokens);

#endif /* _LPC43XX_UART_H_ */
