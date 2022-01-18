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
#include "lpc176x_init.h"
#include <architectures/armv7-m/armv7-m.h>
#include <architectures/armv7-m/debug_cm3.h>


static const char g_memoryMapXml[] = "<?xml version=\"1.0\"?>"
                                     "<!DOCTYPE memory-map PUBLIC \"+//IDN gnu.org//DTD GDB Memory Map V1.0//EN\" \"http://sourceware.org/gdb/gdb-memory-map.dtd\">"
                                     "<memory-map>"
                                       "<memory type=\"flash\" start=\"0x0\" length=\"0x10000\"> <property name=\"blocksize\">0x1000</property></memory>"
                                       "<memory type=\"flash\" start=\"0x10000\" length=\"0x70000\"> <property name=\"blocksize\">0x8000</property></memory>"
                                       "<memory type=\"ram\" start=\"0x10000000\" length=\"0x8000\"> </memory>"
                                       "<memory type=\"rom\" start=\"0x1FFF0000\" length=\"0x2000\"> </memory>"
                                       "<memory type=\"ram\" start=\"0x2007C000\" length=\"0x8000\"> </memory>"
                                       "<memory type=\"ram\" start=\"0x2009C000\" length=\"0x4000\"> </memory>"
                                       "<memory type=\"ram\" start=\"0x22000000\" length=\"0x2000000\"> </memory>"
                                       "<memory type=\"ram\" start=\"0x40000000\" length=\"0x14000\"> </memory>"
                                       "<memory type=\"ram\" start=\"0x40018000\" length=\"0x34000\"> </memory>"
                                       "<memory type=\"ram\" start=\"0x4005C000\" length=\"0x4000\"> </memory>"
                                       "<memory type=\"ram\" start=\"0x40088000\" length=\"0x1C000\"> </memory>"
                                       "<memory type=\"ram\" start=\"0x400A8000\" length=\"0x4000\"> </memory>"
                                       "<memory type=\"ram\" start=\"0x400B0000\" length=\"0x4000\"> </memory>"
                                       "<memory type=\"ram\" start=\"0x400B8000\" length=\"0x8000\"> </memory>"
                                       "<memory type=\"ram\" start=\"0x400FC000\" length=\"0x4000\"> </memory>"
                                       "<memory type=\"ram\" start=\"0x42000000\" length=\"0x2000000\"> </memory>"
                                       "<memory type=\"ram\" start=\"0x50000000\" length=\"0x8000\"> </memory>"
                                       "<memory type=\"ram\" start=\"0x5000C000\" length=\"0x4000\"> </memory>"
                                       "<memory type=\"ram\" start=\"0xE0000000\" length=\"0x100000\"> </memory>"
                                     "</memory-map>";
Lpc176xState mriLpc176xState;


/* Reference this handler in the ASM module to make sure that it gets linked in. */
void UART0_IRQHandler(void);


void mriLpc176x_Init(Token* pParameterTokens)
{
    /* Reference handler in ASM module to make sure that is gets linked in. */
    void (* volatile dummyReference)(void) = UART0_IRQHandler;
    (void)dummyReference;

    __try
        mriCortexMInit(pParameterTokens, 0, CANActivity_IRQn);
    __catch
        __rethrow;

    /* mriCortexInit() sets all interrupts to lower priority than debug monitor. Interrupt for UART used by GDB must be
       elevated to the same level as DebugMon_Handler, so initialize it after calling mriCortexInit().
    */
    mriLpc176xUart_Init(pParameterTokens);
}


uint32_t Platform_GetDeviceMemoryMapXmlSize(void)
{
    return sizeof(g_memoryMapXml) - 1;
}


const char* Platform_GetDeviceMemoryMapXml(void)
{
    return g_memoryMapXml;
}
