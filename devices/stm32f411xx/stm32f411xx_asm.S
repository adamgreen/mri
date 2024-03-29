/* Copyright 2015 Adam Green     (https://github.com/adamgreen/)
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
/* Implementation of STM32F411xx USART1 function to be intercepted and sent to mri instead. */
    .text
    .code 16
    .syntax unified


    .global USART1_IRQHandler
    .weak   USART1_IRQHandler
    .type USART1_IRQHandler, function
    .thumb_func
    /* extern "C" void USART1_IRQHandler(void);
       Override USART1 exceptions and send to mriExceptionHandler.
    */
USART1_IRQHandler:
    b       mriExceptionHandler


    .global USART2_IRQHandler
    .weak   USART2_IRQHandler
    .type USART2_IRQHandler, function
    .thumb_func
    /* extern "C" void USART2_IRQHandler(void);
       Override USART2 exceptions and send to mriExceptionHandler.
    */
USART2_IRQHandler:
    b       mriExceptionHandler

