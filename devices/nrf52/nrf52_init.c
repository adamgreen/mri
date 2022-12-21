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
#include <cmsis/NRF52/nrf52.h>
#include "nrf52_init.h"
#include <core/try_catch.h>
#include <core/platforms.h>
#include <architectures/armv7v8-m/armv7v8-m.h>



static uint32_t parseDebugMonPriorityLevel(Token* pParameterTokens);
void mriNRF52_Init(Token* pParameterTokens)
{
    uint32_t debugMonPriorityLevel = parseDebugMonPriorityLevel(pParameterTokens);

    __try
        mriCortexMInit(pParameterTokens, debugMonPriorityLevel, FPU_IRQn);
    __catch
        __rethrow;

    /* mriCortexInit() sets all interrupts to lower priority than debug monitor. Interrupt for UART used by GDB must be
       elevated to the same level as DebugMon_Handler, so initialize it after calling mriCortexInit().
    */
    mriNRF52Uart_Init(pParameterTokens, debugMonPriorityLevel);
}

static uint32_t parseDebugMonPriorityLevel(Token* pParameterTokens)
{
    static const char priorityLevelPrefix[] = "MRI_PRIORITY=";
    const char*       pMatchingPrefix = NULL;

    /* Default the debug monitor to run at the highest priority. */
    uint32_t          priorityLevel = 0;

    /* Check for user provided override. */
    if ((pMatchingPrefix = Token_MatchingStringPrefix(pParameterTokens, priorityLevelPrefix)) != NULL)
       priorityLevel = uint32FromString(pMatchingPrefix + sizeof(priorityLevelPrefix)-1);
    return priorityLevel;
}


static uint32_t getDecimalDigit(char currChar);
uint32_t mriNRF52_Uint32FromString(const char* pString)
{
    uint32_t value = 0;

    while (*pString)
    {
        uint32_t digit;

        __try
        {
            digit = getDecimalDigit(*pString++);
        }
        __catch
        {
            clearExceptionCode();
            break;
        }

        value = value * 10 + digit;
    }

    return value;
}

static uint32_t getDecimalDigit(char currChar)
{
    if (currChar >= '0' && currChar <= '9')
        return currChar - '0';
    else
        __throw_and_return(invalidDecDigitException, 0);
}

