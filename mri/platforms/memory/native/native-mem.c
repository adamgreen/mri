/* Copyright 2013 Adam Green (http://mbed.org/users/AdamGreen/)

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
/* Routines to access memory on target device when running on target itself. */
#include <platforms.h>

uint32_t Platform_MemRead32(uint32_t address)
{
    return  *(volatile uint32_t*)address;
}

uint16_t Platform_MemRead16(uint32_t address)
{
    return  *(volatile uint16_t*)address;
}

uint8_t Platform_MemRead8(uint32_t address)
{
    return  *(volatile uint8_t*)address;
}

void Platform_MemWrite32(uint32_t address, uint32_t value)
{
    *(volatile uint32_t*)address = value;
}

void Platform_MemWrite16(uint32_t address, uint16_t value)
{
    *(volatile uint16_t*)address = value;
}

void Platform_MemWrite8(uint32_t address, uint8_t value)
{
    *(volatile uint8_t*)address = value;
}
