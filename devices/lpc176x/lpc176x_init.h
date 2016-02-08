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
/* Routines used by mri that are specific to the LPC176x device. */
#ifndef _LPC176X_H_
#define _LPC176X_H_

#include <stdint.h>
#include <token.h>
#include "lpc176x_uart.h"

/* Flags that can be set in Lpc176xState::flags */
#define LPC176X_UART_FLAGS_SHARE        1
#define LPC176X_UART_FLAGS_MANUAL_BAUD  2

/* Flag to indicate whether context will contain FPU registers or not. */
#define MRI_DEVICE_HAS_FPU 0

typedef struct
{
    const UartConfiguration*  pCurrentUart;
    uint32_t                  flags;
} Lpc176xState;

extern Lpc176xState __mriLpc176xState;

void __mriLpc176x_Init(Token* pParameterTokens);

#endif /* _LPC176X_H_ */
