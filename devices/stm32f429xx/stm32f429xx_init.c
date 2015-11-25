/* Copyright 2015 Adam Green (http://mbed.org/users/AdamGreen/)
   Copyright 2015 JaredCJR   (https://github.com/JaredCJR)

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
/* Routines used by mri that are specific to the STM32F429xx device. */
#include <try_catch.h>
#include <platforms.h>
#include "stm32f429xx_init.h"
#include "../../architectures/armv7-m/armv7-m.h"
#include "../../architectures/armv7-m/debug_cm3.h"


static const char g_memoryMapXml[] = "<?xml version=\"1.0\"?>"
                                     "<!DOCTYPE memory-map PUBLIC \"+//IDN gnu.org//DTD GDB Memory Map V1.0//EN\" \"http://sourceware.org/gdb/gdb-memory-map.dtd\">"
                                     "<memory-map>"
                                     "<memory type=\"flash\" start=\"0x08000000\" length=\"0x10000\"> <property name=\"blocksize\">0x4000</property></memory>"
                                     "<memory type=\"flash\" start=\"0x08010000\" length=\"0x10000\"> <property name=\"blocksize\">0x10000</property></memory>"
                                     "<memory type=\"flash\" start=\"0x08020000\" length=\"0xE0000\"> <property name=\"blocksize\">0x20000</property></memory>"
                                     "<memory type=\"flash\" start=\"0x08100000\" length=\"0x10000\"> <property name=\"blocksize\">0x4000</property></memory>"
                                     "<memory type=\"flash\" start=\"0x08110000\" length=\"0x10000\"> <property name=\"blocksize\">0x10000</property></memory>"
                                     "<memory type=\"flash\" start=\"0x08120000\" length=\"0xE0000\"> <property name=\"blocksize\">0x20000</property></memory>"
                                     "<memory type=\"ram\" start=\"0x20000000\" length=\"0x1C000\"> </memory>"
                                     "<memory type=\"ram\" start=\"0x2001C000\" length=\"0x4000\"> </memory>"
                                     "<memory type=\"ram\" start=\"0x20020000\" length=\"0x10000\"> </memory>"
                                     "</memory-map>";
Stm32f429xxState __mriStm32f429xxState;


/* Reference this handler in the ASM module to make sure that it gets linked in. */
void USART1_IRQHandler(void);


static void defaultExternalInterruptsToPriority1(void);
void __mriStm32f429xx_Init(Token* pParameterTokens)
{
    /* Reference handler in ASM module to make sure that is gets linked in. */
    void (* volatile dummyReference)(void) = USART1_IRQHandler;
    (void)dummyReference;

    __try
        __mriCortexMInit(pParameterTokens);
    __catch
        __rethrow;

    defaultExternalInterruptsToPriority1();
    __mriStm32f429xxUart_Init(pParameterTokens);
}

static void defaultExternalInterruptsToPriority1(void)
{
    int irq;
    for (irq = WWDG_IRQn ; irq <= DMA2D_IRQn ; irq++) //Set all priority
	{
		NVIC_SetPriority((IRQn_Type)irq, 1);
	}
}


uint32_t Platform_GetDeviceMemoryMapXmlSize(void)
{
    return sizeof(g_memoryMapXml) - 1;
}


const char* Platform_GetDeviceMemoryMapXml(void)
{
    return g_memoryMapXml;
}
