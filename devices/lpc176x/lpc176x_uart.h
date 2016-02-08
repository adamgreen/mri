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
/* Routines used to provide LPC176x UART functionality to the mri debugger. */
#ifndef _LPC176X_UART_H_
#define _LPC176X_UART_H_

#include <stdint.h>
#include <LPC17xx.h>
#include <token.h>

typedef struct
{
    volatile uint32_t*  pPeripheralClockSelection;
    volatile uint32_t*  pTxPinSelection;
    volatile uint32_t*  pRxPinSelection;
    LPC_UART_TypeDef*   pUartRegisters;
    uint32_t            powerConfigurationBit;
    uint32_t            peripheralClockSelectionBitmask;
    uint32_t            txPinSelectionMask;
    uint32_t            rxPinSelectionMask;
    uint32_t            pinSelectionValue;
} UartConfiguration;

void __mriLpc176xUart_Init(Token* pParameterTokens);

#endif /* _LPC176X_UART_H_ */
