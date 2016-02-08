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
/* Implementation of LPC176x UART0 function to be intercepted and sent to mri instead. */
    .text
    .code 16
    .syntax unified


    .global UART0_IRQHandler
    .weak   UART0_IRQHandler
    .type UART0_IRQHandler, function
    .thumb_func
    /* extern "C" void UART0_IRQHandler(void);
       Override UART0 exceptions and send to __mriExceptionHandler.
    */
UART0_IRQHandler:
    b       __mriExceptionHandler

    .global UART1_IRQHandler
    .weak   UART1_IRQHandler
    .type UART1_IRQHandler, function
    .thumb_func
    /* extern "C" void UART1_IRQHandler(void);
       Override UART1 exceptions and send to __mriExceptionHandler.
    */
UART1_IRQHandler:
    b       __mriExceptionHandler

    .global UART2_IRQHandler
    .weak   UART2_IRQHandler
    .type UART2_IRQHandler, function
    .thumb_func
    /* extern "C" void UART2_IRQHandler(void);
       Override UART2 exceptions and send to __mriExceptionHandler.
    */
UART2_IRQHandler:
    b       __mriExceptionHandler

    .global UART3_IRQHandler
    .weak   UART3_IRQHandler
    .type UART3_IRQHandler, function
    .thumb_func
    /* extern "C" void UART3_IRQHandler(void);
       Override UART3 exceptions and send to __mriExceptionHandler.
    */
UART3_IRQHandler:
    b       __mriExceptionHandler
