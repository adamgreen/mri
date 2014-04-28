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
static int isNotHalfWordAligned(const void* pvMemory);
static void writeBytesToBufferAsHex(Buffer* pBuffer, const void* pv, size_t length);
static int readMemoryWordIntoHexBuffer(Buffer* pBuffer, const void* pvMemory);
static int isNotWordAligned(const void* pvMemory);
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
    uint8_t* p = (uint8_t*) pvMemory;
    
    while (readByteCount-- > 0)
    {
        uint8_t byte;
        
        byte = Platform_MemRead8(p++);
        if (Platform_WasMemoryFaultEncountered())
            return 0;

        Buffer_WriteByteAsHex(pBuffer, byte);
    }

    return 1;
}

static int readMemoryHalfWordIntoHexBuffer(Buffer* pBuffer, const void* pvMemory)
{
    uint16_t value;
    
    if (isNotHalfWordAligned(pvMemory))
        return readMemoryBytesIntoHexBuffer(pBuffer, pvMemory, 2);
        
    value = Platform_MemRead16(pvMemory);
    if (Platform_WasMemoryFaultEncountered())
        return 0;
    writeBytesToBufferAsHex(pBuffer, &value, sizeof(value));

    return 1;
}

static int isNotHalfWordAligned(const void* pvMemory)
{
    return (size_t)pvMemory & 1;
}

static void writeBytesToBufferAsHex(Buffer* pBuffer, const void* pv, size_t length)
{
    uint8_t* pBytes = (uint8_t*)pv;
    while (length--)
        Buffer_WriteByteAsHex(pBuffer, *pBytes++);
}

static int readMemoryWordIntoHexBuffer(Buffer* pBuffer, const void* pvMemory)
{
    uint32_t value;
    
    if (isNotWordAligned(pvMemory))
        return readMemoryBytesIntoHexBuffer(pBuffer, pvMemory, 4);

    value = Platform_MemRead32(pvMemory);
    if (Platform_WasMemoryFaultEncountered())
        return 0;
    writeBytesToBufferAsHex(pBuffer, &value, sizeof(value));

    return 1;
}

static int isNotWordAligned(const void* pvMemory)
{
    return (size_t)pvMemory & 3;
}


static int writeHexBufferToByteMemory(Buffer* pBuffer, void* pvMemory, uint32_t writeByteCount);
static int writeHexBufferToHalfWordMemory(Buffer* pBuffer, void* pvMemory);
static int readBytesFromHexBuffer(Buffer* pBuffer, void* pv, size_t length);
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
    uint8_t* p = (uint8_t*) pvMemory;

    while (writeByteCount-- > 0)
    {
        uint8_t byte;
        
        __try
            byte = Buffer_ReadByteAsHex(pBuffer);
        __catch
            __rethrow_and_return(0);

        Platform_MemWrite8(p++, byte);
        if (Platform_WasMemoryFaultEncountered())
            return 0;
    }

    return 1;
}

static int writeHexBufferToHalfWordMemory(Buffer* pBuffer, void* pvMemory)
{
    uint16_t value;

    if (isNotHalfWordAligned(pvMemory))
        return writeHexBufferToByteMemory(pBuffer, pvMemory, 2);
    
    if (!readBytesFromHexBuffer(pBuffer, &value, sizeof(value)))
        return 0;

    Platform_MemWrite16(pvMemory, value);
    if (Platform_WasMemoryFaultEncountered())
        return 0;

    return 1;
}

static int readBytesFromHexBuffer(Buffer* pBuffer, void* pv, size_t length)
{
    uint8_t* pBytes = (uint8_t*)pv;
    while (length--)
    {
        __try
            *pBytes++ = Buffer_ReadByteAsHex(pBuffer);
        __catch
            __rethrow_and_return(0);
    }
    return 1;
}

static int writeHexBufferToWordMemory(Buffer* pBuffer, void* pvMemory)
{
    uint32_t value;
    
    if (isNotWordAligned(pvMemory))
        return writeHexBufferToByteMemory(pBuffer, pvMemory, 4);

    if (!readBytesFromHexBuffer(pBuffer, &value, sizeof(value)))
        return 0;

    Platform_MemWrite32(pvMemory, value);
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
    uint8_t* p = (uint8_t*) pvMemory;

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

        Platform_MemWrite8(p++, (uint8_t)currChar);
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
    uint16_t value;

    if (isNotHalfWordAligned(pvMemory))
        return writeBinaryBufferToByteMemory(pBuffer, pvMemory, 2);
        
    if (!writeBinaryBufferToByteMemory(pBuffer, &value, sizeof(value)))
        return 0;

    Platform_MemWrite16(pvMemory, value);
    if (Platform_WasMemoryFaultEncountered())
        return 0;

    return 1;
}

static int writeBinaryBufferToWordMemory(Buffer* pBuffer, void* pvMemory) 
{
    uint32_t value;

    if (isNotWordAligned(pvMemory))
        return writeBinaryBufferToByteMemory(pBuffer, pvMemory, 4);

    if (!writeBinaryBufferToByteMemory(pBuffer, &value, sizeof(value)))
        return 0;

    Platform_MemWrite32(pvMemory, value);
    if (Platform_WasMemoryFaultEncountered())
        return 0;

    return 1;
}
