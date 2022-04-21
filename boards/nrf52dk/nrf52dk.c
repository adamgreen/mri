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
/* Routines which expose nRF52-DK specific functionality to the mri debugger. */
#include <string.h>
#include <core/platforms.h>
#include <core/try_catch.h>
#include <devices/nrf52/nrf52_init.h>


static const char g_memoryMapXml[] = "<?xml version=\"1.0\"?>"
                                     "<!DOCTYPE memory-map PUBLIC \"+//IDN gnu.org//DTD GDB Memory Map V1.0//EN\" \"http://sourceware.org/gdb/gdb-memory-map.dtd\">"
                                     "<memory-map>"
                                     "<memory type=\"flash\" start=\"0x00000000\" length=\"0x80000\"> <property name=\"blocksize\">0x1000</property></memory>"
                                     "<memory type=\"ram\" start=\"0x20000000\" length=\"0x10000\"> </memory>"
                                     "</memory-map>";


void Platform_Init(Token* pParameterTokens)
{
    mriNRF52_Init(pParameterTokens);
}


const uint8_t* mriPlatform_GetUid(void)
{
    return NULL;
}


uint32_t mriPlatform_GetUidSize(void)
{
    return 0;
}


uint32_t Platform_GetDeviceMemoryMapXmlSize(void)
{
    return sizeof(g_memoryMapXml) - 1;
}


const char* Platform_GetDeviceMemoryMapXml(void)
{
    return g_memoryMapXml;
}
