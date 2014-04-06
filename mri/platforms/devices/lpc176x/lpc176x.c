/* Copyright 2012 Adam Green (http://mbed.org/users/AdamGreen/)

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
/* Routines used by mri that are specific to the LPC176x device. */
#include <try_catch.h>
#include <platforms.h>
#include "../../architectures/armv7-m/armv7-m.h"
#include "../../architectures/armv7-m/debug_cm3.h"
#include "lpc176x_uart.h"
#include "lpc176x.h"
#include <lpc176xTests.h>


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
Lpc176xState __mriLpc176xState;



static void defaultExternalInterruptsToPriority1(void);
void __mriLpc176x_Init(Token* pParameterTokens)
{
    __try
        __mriCortexMInit(pParameterTokens);
    __catch
        __rethrow;
        
    defaultExternalInterruptsToPriority1();    
    __mriLpc176xUart_Init(pParameterTokens);
}

static void defaultExternalInterruptsToPriority1(void)
{
    static const int CAN_IRQn = 34;
    int              irq;
    
    for (irq = WDT_IRQn ; irq <= CAN_IRQn ; irq++)
        NVIC_SetPriority((IRQn_Type)irq, 1);
}


uint32_t Platform_GetDeviceMemoryMapXmlSize(void)
{
    return sizeof(g_memoryMapXml) - 1;
}


const char* Platform_GetDeviceMemoryMapXml(void)
{
    return g_memoryMapXml;
}
