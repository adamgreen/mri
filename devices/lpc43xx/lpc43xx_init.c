/* Copyright 2020 Adam Green (https://github.com/adamgreen/)

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
/* Routines used by mri that are specific to the LPC176x device. */
#include <core/try_catch.h>
#include <core/platforms.h>
#include "lpc43xx_init.h"
#include <architectures/armv7v8-m/armv7v8-m.h>
#include <architectures/armv7v8-m/debug_cm3.h>


static const char g_memoryMapXml4330[] = "<?xml version=\"1.0\"?>"
                                         "<!DOCTYPE memory-map PUBLIC \"+//IDN gnu.org//DTD GDB Memory Map V1.0//EN\" \"http://sourceware.org/gdb/gdb-memory-map.dtd\">"
                                         "<memory-map>"
                                         "<memory type=\"flash\" start=\"0x14000000\" length=\"0x4000000\"> <property name=\"blocksize\">0x400</property></memory>"
                                         "<memory type=\"ram\" start=\"0x10000000\" length=\"0x20000\"> </memory>"
                                         "<memory type=\"ram\" start=\"0x10080000\" length=\"0x12000\"> </memory>"
                                         "<memory type=\"ram\" start=\"0x20000000\" length=\"0x8000\"> </memory>"
                                         "<memory type=\"ram\" start=\"0x20008000\" length=\"0x8000\"> </memory>"
                                         "</memory-map>";
static const char g_memoryMapXml4337[] = "<?xml version=\"1.0\"?>"
                                         "<!DOCTYPE memory-map PUBLIC \"+//IDN gnu.org//DTD GDB Memory Map V1.0//EN\" \"http://sourceware.org/gdb/gdb-memory-map.dtd\">"
                                         "<memory-map>"
                                         "<memory type=\"flash\" start=\"0x1A000000\" length=\"0x80000\"> <property name=\"blocksize\">0x400</property></memory>"
                                         "<memory type=\"flash\" start=\"0x1B000000\" length=\"0x80000\"> <property name=\"blocksize\">0x400</property></memory>"
                                         "<memory type=\"ram\" start=\"0x10000000\" length=\"0x8000\"> </memory>"
                                         "<memory type=\"ram\" start=\"0x10080000\" length=\"0xA000\"> </memory>"
                                         "<memory type=\"ram\" start=\"0x20000000\" length=\"0x8000\"> </memory>"
                                         "<memory type=\"ram\" start=\"0x20008000\" length=\"0x8000\"> </memory>"
                                         "</memory-map>";
Lpc43xxState mriLpc43xxState;



/* Reference this handler in the ASM module to make sure that it gets linked in. */
void USART0_IRQHandler(void);


void mriLpc43xx_Init(Token* pParameterTokens)
{
    /* Reference handler in ASM module to make sure that is gets linked in. */
    void (* volatile dummyReference)(void) = USART0_IRQHandler;
    (void)dummyReference;

    __try
        mriCortexMInit(pParameterTokens, 0, QEI_IRQn);
    __catch
        __rethrow;

    /* mriCortexInit() sets all interrupts to lower priority than debug monitor. Interrupt for UART used by GDB must be
       elevated to the same level as DebugMon_Handler, so initialize it after calling mriCortexInit().
    */
    mriLpc43xxUart_Init(pParameterTokens);
}


static int isLpc4337(void);
uint32_t Platform_GetDeviceMemoryMapXmlSize(void)
{
    if (isLpc4337())
        return sizeof(g_memoryMapXml4337) - 1;
    else
        return sizeof(g_memoryMapXml4330) - 1;
}

static int isLpc4337(void)
{
    /* Code running on the LPC4337 will be in internal FLASH which is at higher address than LPC4330 SPIFI FLASH. */
    return ((uint32_t)mriLpc43xx_Init >= 0x1A000000);
}


const char* Platform_GetDeviceMemoryMapXml(void)
{
    if (isLpc4337())
        return g_memoryMapXml4337;
    else
        return g_memoryMapXml4330;
}
