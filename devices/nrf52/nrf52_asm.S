/* Copyright 2022 Adam Green (https://github.com/adamgreen/)

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
/* Implementation of nRF52xxx UART0 function to be intercepted and sent to mri instead. */
/* NOTE: It may be necessary to pull this source file into a project using MRI and build it there directly rather than
         pulling in from this library so that it properly replaces the weak version from another .o file.
*/
    .text
    .code 16
    .syntax unified


    .global UARTE0_UART0_IRQHandler
    .type UARTE0_UART0_IRQHandler, function
    .thumb_func
    /* extern "C" void UARTE0_UART0_IRQHandler(void);
       Override UART0 exceptions and send to mriExceptionHandler.
    */
UARTE0_UART0_IRQHandler:
    b.w       mriExceptionHandler

