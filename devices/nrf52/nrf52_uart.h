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
/* Routines used to provide NRF52xxx UART functionality to the mri debugger. */
#ifndef NRF52_UART_H_
#define NRF52_UART_H_

#include <stdint.h>
#include <core/token.h>

void mriNRF52Uart_Init(Token* pParameterTokens, uint32_t debugMonPriorityLevel);

#endif /* NRF52_UART_H_ */
