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
/* Routines to read/write memory and detect any faults that might occur while attempting to do so. */
#include <core/platforms.h>
#include <core/memory.h>


static uintmri_t readMemoryBytesIntoHexBuffer(Buffer* pBuffer, uintmri_t address, uintmri_t readByteCount);
static uintmri_t readMemoryHalfWordIntoHexBuffer(Buffer* pBuffer, uintmri_t address);
static int isNotHalfWordAligned(uintmri_t address);
static void writeBytesToBufferAsHex(Buffer* pBuffer, const void* pv, size_t length);
static uintmri_t readMemoryWordIntoHexBuffer(Buffer* pBuffer, uintmri_t address);
static int isNotWordAligned(uintmri_t address);
static uintmri_t readMemoryDoubleWordIntoHexBuffer(Buffer* pBuffer, uintmri_t address);
static int isNot64BitAligned(uintmri_t address);
uintmri_t ReadMemoryIntoHexBuffer(Buffer* pBuffer, uintmri_t address, uintmri_t readByteCount)
{
    switch (readByteCount)
    {
    case 2:
        return readMemoryHalfWordIntoHexBuffer(pBuffer, address);
    case 4:
        return readMemoryWordIntoHexBuffer(pBuffer, address);
    case 8:
        return readMemoryDoubleWordIntoHexBuffer(pBuffer, address);
    default:
        return readMemoryBytesIntoHexBuffer(pBuffer, address, readByteCount);
    }
}

static uintmri_t readMemoryBytesIntoHexBuffer(Buffer* pBuffer, uintmri_t address, uintmri_t readByteCount)
{
    uintmri_t byteCount = 0;

    while (readByteCount-- > 0)
    {
        uint8_t byte;

        byte = Platform_MemRead8(address++);
        if (Platform_WasMemoryFaultEncountered())
            break;

        Buffer_WriteByteAsHex(pBuffer, byte);
        byteCount++;
    }

    return byteCount;
}

static uintmri_t readMemoryHalfWordIntoHexBuffer(Buffer* pBuffer, uintmri_t address)
{
    uint16_t value;

    if (isNotHalfWordAligned(address))
        return readMemoryBytesIntoHexBuffer(pBuffer, address, sizeof(uint16_t));

    value = Platform_MemRead16(address);
    if (Platform_WasMemoryFaultEncountered())
        return 0;
    writeBytesToBufferAsHex(pBuffer, &value, sizeof(value));

    return sizeof(value);
}

static int isNotHalfWordAligned(uintmri_t address)
{
    return address & 1;
}

static void writeBytesToBufferAsHex(Buffer* pBuffer, const void* pv, size_t length)
{
    uint8_t* pBytes = (uint8_t*)pv;
    while (length--)
        Buffer_WriteByteAsHex(pBuffer, *pBytes++);
}

static uintmri_t readMemoryWordIntoHexBuffer(Buffer* pBuffer, uintmri_t address)
{
    uint32_t value;

    if (isNotWordAligned(address))
        return readMemoryBytesIntoHexBuffer(pBuffer, address, sizeof(uint32_t));

    value = Platform_MemRead32(address);
    if (Platform_WasMemoryFaultEncountered())
        return 0;
    writeBytesToBufferAsHex(pBuffer, &value, sizeof(value));

    return sizeof(value);
}

static int isNotWordAligned(uintmri_t address)
{
    return address & 3;
}

static uintmri_t readMemoryDoubleWordIntoHexBuffer(Buffer* pBuffer, uintmri_t address)
{
    if (sizeof(uintmri_t) >= 8)
    {
        uint64_t value;

        if (isNot64BitAligned(address))
            return readMemoryBytesIntoHexBuffer(pBuffer, address, sizeof(uint64_t));

        value = Platform_MemRead64(address);
        if (Platform_WasMemoryFaultEncountered())
            return 0;
        writeBytesToBufferAsHex(pBuffer, &value, sizeof(value));

        return sizeof(value);
    }
    else
    {
        return readMemoryBytesIntoHexBuffer(pBuffer, address, sizeof(uint64_t));
    }
}

static int isNot64BitAligned(uintmri_t address)
{
    return address & 7;
}


static int writeHexBufferToByteMemory(Buffer* pBuffer, uintmri_t address, uintmri_t writeByteCount);
static int writeHexBufferToHalfWordMemory(Buffer* pBuffer, uintmri_t address);
static int readBytesFromHexBuffer(Buffer* pBuffer, void* pv, size_t length);
static int writeHexBufferToWordMemory(Buffer* pBuffer, uintmri_t address);
static int writeHexBufferToDoubleWordMemory(Buffer* pBuffer, uintmri_t address);
int WriteHexBufferToMemory(Buffer* pBuffer, uintmri_t address, uintmri_t writeByteCount)
{
    switch (writeByteCount)
    {
    case 2:
        return writeHexBufferToHalfWordMemory(pBuffer, address);
    case 4:
        return writeHexBufferToWordMemory(pBuffer, address);
    case 8:
        return writeHexBufferToDoubleWordMemory(pBuffer, address);
    default:
        return writeHexBufferToByteMemory(pBuffer, address, writeByteCount);
    }
}

static int writeHexBufferToByteMemory(Buffer* pBuffer, uintmri_t address, uintmri_t writeByteCount)
{
    while (writeByteCount-- > 0)
    {
        uint8_t byte;

        __try
            byte = Buffer_ReadByteAsHex(pBuffer);
        __catch
            __rethrow_and_return(0);

        Platform_MemWrite8(address++, byte);
        if (Platform_WasMemoryFaultEncountered())
            return 0;
    }

    return 1;
}

static int writeHexBufferToHalfWordMemory(Buffer* pBuffer, uintmri_t address)
{
    uint16_t value;

    if (isNotHalfWordAligned(address))
        return writeHexBufferToByteMemory(pBuffer, address, sizeof(uint16_t));

    if (!readBytesFromHexBuffer(pBuffer, &value, sizeof(value)))
        return 0;

    Platform_MemWrite16(address, value);
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

static int writeHexBufferToWordMemory(Buffer* pBuffer, uintmri_t address)
{
    uint32_t value;

    if (isNotWordAligned(address))
        return writeHexBufferToByteMemory(pBuffer, address, sizeof(uint32_t));

    if (!readBytesFromHexBuffer(pBuffer, &value, sizeof(value)))
        return 0;

    Platform_MemWrite32(address, value);
    if (Platform_WasMemoryFaultEncountered())
        return 0;

    return 1;
}

static int writeHexBufferToDoubleWordMemory(Buffer* pBuffer, uintmri_t address)
{
    if (sizeof(uintmri_t) >= 8)
    {
        uint64_t value;

        if (isNot64BitAligned(address))
            return writeHexBufferToByteMemory(pBuffer, address, sizeof(uint64_t));

        if (!readBytesFromHexBuffer(pBuffer, &value, sizeof(value)))
            return 0;

        Platform_MemWrite64(address, value);
        if (Platform_WasMemoryFaultEncountered())
            return 0;

        return 1;
    }
    else
    {
        return writeHexBufferToByteMemory(pBuffer, address, sizeof(uint64_t));
    }
}


static int  writeBinaryBufferToByteMemory(Buffer*  pBuffer, uintmri_t address, uintmri_t writeByteCount);
static int  writeBinaryBufferToHalfWordMemory(Buffer* pBuffer, uintmri_t address);
static int readBytesFromBinaryBuffer(Buffer*  pBuffer, void* pvMemory, uintmri_t writeByteCount);
static int  writeBinaryBufferToWordMemory(Buffer* pBuffer, uintmri_t address);
static int  writeBinaryBufferToDoubleWordMemory(Buffer* pBuffer, uintmri_t address);
int WriteBinaryBufferToMemory(Buffer* pBuffer, uintmri_t address, uintmri_t writeByteCount)
{
    switch (writeByteCount)
    {
    case 2:
        return writeBinaryBufferToHalfWordMemory(pBuffer, address);
    case 4:
        return writeBinaryBufferToWordMemory(pBuffer, address);
    case 8:
        return writeBinaryBufferToDoubleWordMemory(pBuffer, address);
    default:
        return writeBinaryBufferToByteMemory(pBuffer, address, writeByteCount);
    }
}

static int writeBinaryBufferToByteMemory(Buffer*  pBuffer, uintmri_t address, uintmri_t writeByteCount)
{
    while (writeByteCount-- > 0)
    {
        char currChar;

        __try
            currChar = Buffer_ReadChar(pBuffer);
        __catch
            __rethrow_and_return(0);

        Platform_MemWrite8(address++, (uint8_t)currChar);
        if (Platform_WasMemoryFaultEncountered())
            return 0;
    }

    return 1;
}

static int writeBinaryBufferToHalfWordMemory(Buffer* pBuffer, uintmri_t address)
{
    uint16_t value;

    if (isNotHalfWordAligned(address))
        return writeBinaryBufferToByteMemory(pBuffer, address, sizeof(uint16_t));

    if (!readBytesFromBinaryBuffer(pBuffer, &value, sizeof(value)))
        return 0;

    Platform_MemWrite16(address, value);
    if (Platform_WasMemoryFaultEncountered())
        return 0;

    return 1;
}

static int readBytesFromBinaryBuffer(Buffer*  pBuffer, void* pvMemory, uintmri_t writeByteCount)
{
    uint8_t* p = (uint8_t*) pvMemory;

    while (writeByteCount-- > 0)
    {
        __try
            *p++ = Buffer_ReadChar(pBuffer);
        __catch
            __rethrow_and_return(0);
    }

    return 1;
}

static int writeBinaryBufferToWordMemory(Buffer* pBuffer, uintmri_t address)
{
    uint32_t value;

    if (isNotWordAligned(address))
        return writeBinaryBufferToByteMemory(pBuffer, address, sizeof(uint32_t));

    if (!readBytesFromBinaryBuffer(pBuffer, &value, sizeof(value)))
        return 0;

    Platform_MemWrite32(address, value);
    if (Platform_WasMemoryFaultEncountered())
        return 0;

    return 1;
}

static int writeBinaryBufferToDoubleWordMemory(Buffer* pBuffer, uintmri_t address)
{
    if (sizeof(uintmri_t) >= 8)
    {
        uint64_t value;

        if (isNot64BitAligned(address))
            return writeBinaryBufferToByteMemory(pBuffer, address, sizeof(uint64_t));

        if (!readBytesFromBinaryBuffer(pBuffer, &value, sizeof(value)))
            return 0;

        Platform_MemWrite64(address, value);
        if (Platform_WasMemoryFaultEncountered())
            return 0;

        return 1;
    }
    else
    {
        return writeBinaryBufferToByteMemory(pBuffer, address, sizeof(uint64_t));
    }
}
