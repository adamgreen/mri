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
/* Routines used by mri that are specific to nRF52xxx devices. */
#ifndef NRF52_H_
#define NRF52_H_

#include <stdint.h>
#include <core/token.h>
#include "nrf52_uart.h"

/* Flag to indicate whether context will contain FPU registers or not. */
#define MRI_DEVICE_HAS_FPU 1


void mriNRF52_Init(Token* pParameterTokens);

uint32_t mriNRF52_Uint32FromString(const char* pString);

#define uint32FromString mriNRF52_Uint32FromString

#endif /* NRF52_H_ */
