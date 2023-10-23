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
/*  'Class' which represents a scatter gather list of registers so that blocks of them can be pulled from various
    locations on the stack and they don't all need to be placed in one contiguous place in memory.
*/
#include <core/context.h>
#include <core/try_catch.h>


void Context_Init(MriContext* pThis, ContextSection* pSections, size_t sectionCount)
{
    pThis->pSections = pSections;
    pThis->sectionCount = sectionCount;
}

size_t Context_Count(MriContext* pThis)
{
    size_t i;
    size_t count = 0;
    for (i = 0 ; i < pThis->sectionCount ; i++)
    {
        count += pThis->pSections[i].count;
    }
    return count;
}

uintmri_t Context_Get(const MriContext* pThis, size_t index)
{
    size_t i;
    size_t count = 0;
    size_t base = 0;
    for (i = 0 ; i < pThis->sectionCount ; i++)
    {
        base = count;
        count += pThis->pSections[i].count;
        if (index < count)
        {
            return pThis->pSections[i].pValues[index - base];
        }
    }
    __throw_and_return(bufferOverrunException, 0);
}

void Context_Set(MriContext* pThis, size_t index, uintmri_t newValue)
{
    size_t i;
    size_t count = 0;
    size_t base = 0;
    for (i = 0 ; i < pThis->sectionCount ; i++)
    {
        base = count;
        count += pThis->pSections[i].count;
        if (index < count)
        {
            pThis->pSections[i].pValues[index - base] = newValue;
            return;
        }
    }
    __throw(bufferOverrunException);
}


static void writeBytesToBufferAsHex(Buffer* pBuffer, void* pBytes, size_t byteCount);
void Context_CopyToBuffer(MriContext* pThis, Buffer* pBuffer)
{
    size_t count = Context_Count(pThis);
    size_t i;

    for (i = 0 ; i < count ; i++)
    {
        uintmri_t reg = Context_Get(pThis, i);
        writeBytesToBufferAsHex(pBuffer, &reg, sizeof(reg));
    }
}

static void writeBytesToBufferAsHex(Buffer* pBuffer, void* pBytes, size_t byteCount)
{
    uint8_t* pByte = (uint8_t*)pBytes;
    while (byteCount--)
        Buffer_WriteByteAsHex(pBuffer, *pByte++);
}


static void readBytesFromBufferAsHex(Buffer* pBuffer, void* pBytes, size_t byteCount);
void Context_CopyFromBuffer(MriContext* pThis, Buffer* pBuffer)
{
    size_t count = Context_Count(pThis);
    size_t i;

    for (i = 0 ; i < count ; i++) {
        uintmri_t reg;
        readBytesFromBufferAsHex(pBuffer, &reg, sizeof(reg));
        Context_Set(pThis, i, reg);
    }
}

static void readBytesFromBufferAsHex(Buffer* pBuffer, void* pBytes, size_t byteCount)
{
    uint8_t* pByte = (uint8_t*)pBytes;
    while (byteCount--)
        *pByte++ = Buffer_ReadByteAsHex(pBuffer);
}