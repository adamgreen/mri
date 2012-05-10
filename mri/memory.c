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
/* Routines to read/write memory and detect any faults that might occur while attempting to do so. */
#include "platforms.h"
#include "memory.h"


static int readMemoryBytesIntoHexBuffer(Buffer* pBuffer, const void*  pvMemory, uint32_t readByteCount);
static int readMemoryHalfWordIntoHexBuffer(Buffer* pBuffer, const void*  pvMemory);
static int readMemoryWordIntoHexBuffer(Buffer* pBuffer, const void* pvMemory);
int ReadMemoryIntoHexBuffer(Buffer* pBuffer, const void* pvMemory, uint32_t readByteCount)
{
    switch (readByteCount)
    {
    case 2:
        return readMemoryHalfWordIntoHexBuffer(pBuffer, pvMemory);
    case 4:
        return readMemoryWordIntoHexBuffer(pBuffer, pvMemory);
    default:
        return readMemoryBytesIntoHexBuffer(pBuffer, pvMemory, readByteCount);
    }
}

static int readMemoryBytesIntoHexBuffer(Buffer* pBuffer, const void*  pvMemory, uint32_t readByteCount)
{
    const volatile uint8_t* pMemory = (const volatile uint8_t*) pvMemory;
    
    while (readByteCount-- > 0)
    {
        uint8_t byte;
        
        byte = *pMemory++;
        if (Platform_WasMemoryFaultEncountered())
            return 0;

        Buffer_WriteByteAsHex(pBuffer, byte);
    }

    return 1;
}

static int readMemoryHalfWordIntoHexBuffer(Buffer* pBuffer, const void* pvMemory)
{
    const volatile uint16_t* pMemory = (const volatile uint16_t*) pvMemory;
    uint16_t                 value;
    
    value = *pMemory;
    if (Platform_WasMemoryFaultEncountered())
        return 0;
    readMemoryBytesIntoHexBuffer(pBuffer, &value, sizeof(value));

    return 1;
}

static int readMemoryWordIntoHexBuffer(Buffer* pBuffer, const void* pvMemory)
{
    const volatile uint32_t* pMemory = (const volatile uint32_t*) pvMemory;
    uint32_t                 value;
    
    value = *pMemory;
    if (Platform_WasMemoryFaultEncountered())
        return 0;
    readMemoryBytesIntoHexBuffer(pBuffer, &value, sizeof(value));

    return 1;
}


static int writeHexBufferToByteMemory(Buffer* pBuffer, void* pvMemory, uint32_t writeByteCount);
static int writeHexBufferToHalfWordMemory(Buffer* pBuffer, void* pvMemory);
static int writeHexBufferToWordMemory(Buffer* pBuffer, void* pvMemory);
int WriteHexBufferToMemory(Buffer* pBuffer, void* pvMemory, uint32_t writeByteCount)
{
    switch (writeByteCount)
    {
    case 2:
        return writeHexBufferToHalfWordMemory(pBuffer, pvMemory);
    case 4:
        return writeHexBufferToWordMemory(pBuffer, pvMemory);
    default:
        return writeHexBufferToByteMemory(pBuffer, pvMemory, writeByteCount);
    }
}

static int writeHexBufferToByteMemory(Buffer* pBuffer, void* pvMemory, uint32_t writeByteCount)
{
    volatile uint8_t* pMemory = (volatile uint8_t*)pvMemory;

    while (writeByteCount-- > 0)
    {
        uint8_t byte;
        
        __try
            byte = Buffer_ReadByteAsHex(pBuffer);
        __catch
            __rethrow_and_return(0);

        *pMemory++ = byte;
        if (Platform_WasMemoryFaultEncountered())
            return 0;
    }

    return 1;
}

static int writeHexBufferToHalfWordMemory(Buffer* pBuffer, void* pvMemory)
{
    volatile uint16_t* pMemory = (volatile uint16_t*)pvMemory;
    uint16_t           value;

    if (!writeHexBufferToByteMemory(pBuffer, &value, sizeof(value)))
        return 0;

    *pMemory = value;
    if (Platform_WasMemoryFaultEncountered())
        return 0;

    return 1;
}

static int writeHexBufferToWordMemory(Buffer* pBuffer, void* pvMemory)
{
    volatile uint32_t* pMemory = (volatile uint32_t*)pvMemory;
    uint32_t           value;

    if (!writeHexBufferToByteMemory(pBuffer, &value, sizeof(value)))
        return 0;

    *pMemory = value;
    if (Platform_WasMemoryFaultEncountered())
        return 0;

    return 1;
}


static int  writeBinaryBufferToByteMemory(Buffer*  pBuffer, void* pvMemory, uint32_t writeByteCount);
static char unescapeCharIfNecessary(Buffer* pBuffer, char currentChar);
static int  isEscapePrefixChar(char charToCheck);
static char readNextCharAndUnescape(Buffer* pBuffer);
static char unescapeByte(char charToUnescape);
static int  writeBinaryBufferToHalfWordMemory(Buffer* pBuffer, void* pvMemory);
static int  writeBinaryBufferToWordMemory(Buffer* pBuffer, void* pvMemory);
int WriteBinaryBufferToMemory(Buffer* pBuffer, void* pvMemory, uint32_t writeByteCount)
{
    switch (writeByteCount)
    {
    case 2:
        return writeBinaryBufferToHalfWordMemory(pBuffer, pvMemory);
    case 4:
        return writeBinaryBufferToWordMemory(pBuffer, pvMemory);
    default:
        return writeBinaryBufferToByteMemory(pBuffer, pvMemory, writeByteCount);
    }
}

static int writeBinaryBufferToByteMemory(Buffer*  pBuffer, void* pvMemory, uint32_t writeByteCount)
{
    volatile uint8_t* pMemory = (volatile uint8_t*)pvMemory;

    while (writeByteCount-- > 0)
    {
        char currChar;
    
        __try
        {
            __throwing_func( currChar = Buffer_ReadChar(pBuffer) );
            __throwing_func( currChar = unescapeCharIfNecessary(pBuffer, currChar) );
        }
        __catch
        {
            __rethrow_and_return(0);
        }

        *pMemory++ = (uint8_t)currChar;
        if (Platform_WasMemoryFaultEncountered())
            return 0;
    }

    return 1;
}

static char unescapeCharIfNecessary(Buffer* pBuffer, char currentChar)
{
    if (isEscapePrefixChar(currentChar))
        return readNextCharAndUnescape(pBuffer);
    
    return currentChar;
}

static int isEscapePrefixChar(char charToCheck)
{
    return charToCheck == '}';
}

static char readNextCharAndUnescape(Buffer* pBuffer)
{
    char nextChar;
    
    __try
        nextChar = Buffer_ReadChar(pBuffer);
    __catch
        __rethrow_and_return('\0');

    return unescapeByte(nextChar);
}

static char unescapeByte(char charToUnescape)
{
    return charToUnescape ^ 0x20;
}

static int writeBinaryBufferToHalfWordMemory(Buffer* pBuffer, void* pvMemory) 
{
    volatile uint16_t* pMemory = (volatile uint16_t*)pvMemory;
    uint16_t           value;

    if (!writeBinaryBufferToByteMemory(pBuffer, &value, sizeof(value)))
        return 0;

    *pMemory = value;
    if (Platform_WasMemoryFaultEncountered())
        return 0;

    return 1;
}

static int writeBinaryBufferToWordMemory(Buffer* pBuffer, void* pvMemory) 
{
    volatile uint32_t* pMemory = (volatile uint32_t*)pvMemory;
    uint32_t           value;

    if (!writeBinaryBufferToByteMemory(pBuffer, &value, sizeof(value)))
        return 0;

    *pMemory = value;
    if (Platform_WasMemoryFaultEncountered())
        return 0;

    return 1;
}
