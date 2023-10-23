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
#ifndef CONTEXT_H_
#define CONTEXT_H_

#include <stdint.h>
#include <core/buffer.h>

typedef struct
{
    uintmri_t* pValues;
    size_t     count;
} ContextSection;

typedef struct
{
    ContextSection* pSections;
    size_t          sectionCount;
} MriContext;

/* Real name of functions are in mri namespace. */
void      mriContext_Init(MriContext* pThis, ContextSection* pSections, size_t sectionCount);
size_t    mriContext_Count(MriContext* pThis);
uintmri_t mriContext_Get(const MriContext* pThis, size_t index);
void      mriContext_Set(MriContext* pThis, size_t index, uintmri_t newValue);
void      mriContext_CopyToBuffer(MriContext* pThis, Buffer* pBuffer);
void      mriContext_CopyFromBuffer(MriContext* pThis, Buffer* pBuffer);

/* Macroes which allow code to drop the mri namespace prefix. */
#define Context_Init            mriContext_Init
#define Context_Count           mriContext_Count
#define Context_Get             mriContext_Get
#define Context_Set             mriContext_Set
#define Context_CopyToBuffer    mriContext_CopyToBuffer
#define Context_CopyFromBuffer  mriContext_CopyFromBuffer

#endif /* CONTEXT_H_ */
