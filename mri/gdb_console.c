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
#include <string.h>
#include "buffer.h"
#include "platforms.h"
#include "core.h"
#include "memory.h"
#include "gdb_console.h"


static void writeStringToSharedCommChannel(const char* pString);
static void writeStringToExclusiveGdbCommChannel(const char* pString);
void WriteStringToGdbConsole(const char* pString)
{
    if (Platform_CommSharingWithApplication() && IsFirstException())
        writeStringToSharedCommChannel(pString);
    else
        writeStringToExclusiveGdbCommChannel(pString);
}

static void writeStringToSharedCommChannel(const char* pString)
{
    while(*pString)
        Platform_CommSendChar(*pString++);
}

/* Send the 'O' command to gdb to output text to its console.

    Command Format: OXX...
    Where XX is the hexadecimal representation of each character in the string to be sent to the gdb console.
*/
static void writeStringToExclusiveGdbCommChannel(const char* pString)
{
    Buffer* pBuffer = GetInitializedBuffer();

    Buffer_WriteChar(pBuffer, 'O');
    ReadMemoryIntoHexBuffer(pBuffer, pString, strlen(pString));

    if (!Buffer_OverrunDetected(pBuffer))
        SendPacketToGdb();
}


void WriteHexValueToGdbConsole(uint32_t Value)
{
    Buffer BufferObject;
    char   StringBuffer[11];
    
    Buffer_Init(&BufferObject, StringBuffer, sizeof(StringBuffer));
    Buffer_WriteString(&BufferObject, "0x");
    Buffer_WriteUIntegerAsHex(&BufferObject, Value);
    Buffer_WriteChar(&BufferObject, '\0');
    
    WriteStringToGdbConsole(StringBuffer);
}
