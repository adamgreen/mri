/* Copyright 2020 Adam Green     (https://github.com/adamgreen/)
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
/* Routines used by mri that are specific to the STM32F411xx device. */
#include <core/try_catch.h>
#include <core/platforms.h>
#include "stm32f411xx_init.h"
#include <architectures/armv7-m/armv7-m.h>
#include <architectures/armv7-m/debug_cm3.h>


static const char g_memoryMapXml[] =
"<?xml version=\"1.0\"?>"
"<!DOCTYPE memory-map PUBLIC \"+//IDN gnu.org//DTD GDB Memory Map V1.0//EN\" \"http://sourceware.org/gdb/gdb-memory-map.dtd\">"
"<memory-map>"
"<memory type=\"flash\" start=\"0x08000000\" length=\"0x10000\"> <property name=\"blocksize\">0x4000</property></memory>"
"<memory type=\"flash\" start=\"0x08010000\" length=\"0x10000\"> <property name=\"blocksize\">0x10000</property></memory>"
"<memory type=\"flash\" start=\"0x08020000\" length=\"0x60000\"> <property name=\"blocksize\">0x20000</property></memory>"
"<memory type=\"ram\" start=\"0x20000000\" length=\"0x20000\"> </memory>"
"</memory-map>";
Stm32f411xxState mriStm32f411xxState;


void mriStm32f411xx_Init(Token* pParameterTokens)
{
    __try
        mriCortexMInit(pParameterTokens, 0, SPI5_IRQn);
    __catch
        __rethrow;

    /* mriCortexInit() sets all interrupts to lower priority than debug monitor. Interrupt for UART used by GDB must be
       elevated to the same level as DebugMon_Handler, so initialize it after calling mriCortexInit().
    */
    mriStm32f411xxUart_Init(pParameterTokens);
}



uint32_t Platform_GetDeviceMemoryMapXmlSize(void)
{
    return sizeof(g_memoryMapXml) - 1;
}


const char* Platform_GetDeviceMemoryMapXml(void)
{
    return g_memoryMapXml;
}
