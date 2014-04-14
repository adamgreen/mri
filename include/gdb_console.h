/* Copyright 2014 Adam Green (http://mbed.org/users/AdamGreen/)

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
/* Routines to output text to stdout on the gdb console. */
#ifndef _GDB_CONSOLE_H_
#define _GDB_CONSOLE_H_

#include <stdint.h>

/* Real name of functions are in __mri namespace. */
void __mriGdbConsole_WriteString(const char* pString);
void __mriGdbConsole_WriteHexValue(uint32_t value);

/* Macroes which allow code to drop the __mri namespace prefix. */
#define WriteStringToGdbConsole     __mriGdbConsole_WriteString
#define WriteHexValueToGdbConsole   __mriGdbConsole_WriteHexValue

#endif /* _GDB_CONSOLE_H_ */
