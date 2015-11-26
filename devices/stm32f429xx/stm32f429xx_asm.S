/* Copyright 2015 Adam Green     (http://mbed.org/users/AdamGreen/)
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
/* Implementation of STM32F429xx USART1 function to be intercepted and sent to mri instead. */
    .text
    .code 16
    .syntax unified


    .global USART1_IRQHandler
    .weak   USART1_IRQHandler
    .type USART1_IRQHandler, function
    .thumb_func
    /* extern "C" void USART1_IRQHandler(void);
       Override USART1 exceptions and send to __mriExceptionHandler.
    */
USART1_IRQHandler:
    b       __mriExceptionHandler


    .global USART2_IRQHandler
    .weak   USART2_IRQHandler
    .type USART2_IRQHandler, function
    .thumb_func
    /* extern "C" void USART2_IRQHandler(void);
       Override USART2 exceptions and send to __mriExceptionHandler.
    */
USART2_IRQHandler:
    b       __mriExceptionHandler


    .global USART3_IRQHandler
    .weak   USART3_IRQHandler
    .type USART3_IRQHandler, function
    .thumb_func
    /* extern "C" void USART3_IRQHandler(void);
       Override USART3 exceptions and send to __mriExceptionHandler.
    */
USART3_IRQHandler:
    b       __mriExceptionHandler

