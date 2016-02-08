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
/* Routines to read/write memory and detect any faults that might occur while attempting to do so. */
#ifndef _MEMORY_H_
#define _MEMORY_H_

#include <stdint.h>
#include "buffer.h"

/* Real name of functions are in __mri namespace. */
int __mriMem_ReadMemoryIntoHexBuffer(Buffer* pBuffer, const void* pvMemory, uint32_t readByteCount);
int __mriMem_WriteHexBufferToMemory(Buffer* pBuffer, void* pvMemory, uint32_t writeByteCount);
int __mriMem_WriteBinaryBufferToMemory(Buffer* pBuffer, void* pvMemory, uint32_t writeByteCount);

/* Macroes which allow code to drop the __mri namespace prefix. */
#define ReadMemoryIntoHexBuffer     __mriMem_ReadMemoryIntoHexBuffer
#define WriteHexBufferToMemory      __mriMem_WriteHexBufferToMemory
#define WriteBinaryBufferToMemory   __mriMem_WriteBinaryBufferToMemory

#endif /* _MEMORY_H_ */
