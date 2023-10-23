/* Copyright 2023 Adam Green (https://github.com/adamgreen/)

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
/* Routines to access memory on target device when running on target itself. */
#include <core/platforms.h>

uint64_t Platform_MemRead64(uintmri_t address)
{
    return  *(volatile const uint64_t*)address;
}

uint32_t Platform_MemRead32(uintmri_t address)
{
    return  *(volatile const uint32_t*)address;
}

uint16_t Platform_MemRead16(uintmri_t address)
{
    return  *(volatile const uint16_t*)address;
}

uint8_t Platform_MemRead8(uintmri_t address)
{
    return  *(volatile const uint8_t*)address;
}

void Platform_MemWrite64(uintmri_t address, uint64_t value)
{
    *(volatile uint64_t*)address = value;
}

void Platform_MemWrite32(uintmri_t address, uint32_t value)
{
    *(volatile uint32_t*)address = value;
}

void Platform_MemWrite16(uintmri_t address, uint16_t value)
{
    *(volatile uint16_t*)address = value;
}

void Platform_MemWrite8(uintmri_t address, uint8_t value)
{
    *(volatile uint8_t*)address = value;
}
