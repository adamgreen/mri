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
/*  'Class' which represents a text buffer.  Has routines to both extract and inject strings of various types into the
    buffer while verifying that no overflow takes place. */
#ifndef _BUFFER_H_
#define _BUFFER_H_

#include <stddef.h>
#include <stdint.h>

typedef struct
{
    char*   pStart;
    char*   pEnd;
    char*   pCurrent;
} Buffer;

/* Real name of functions are in __mri namespace. */
void     __mriBuffer_Init(Buffer* pBuffer, char* pBufferStart, size_t bufferSize);
void     __mriBuffer_Reset(Buffer* pBuffer);
void     __mriBuffer_SetEndOfBuffer(Buffer* pBuffer);
size_t   __mriBuffer_BytesLeft(Buffer* pBuffer);
int      __mriBuffer_OverrunDetected(Buffer* pBuffer);
size_t   __mriBuffer_GetLength(Buffer* pBuffer);
char*    __mriBuffer_GetArray(Buffer* pBuffer);
void     __mriBuffer_WriteChar(Buffer* pBuffer, char character);
char     __mriBuffer_ReadChar(Buffer* pBuffer);
void     __mriBuffer_WriteByteAsHex(Buffer* pBuffer, uint8_t byte);
uint8_t  __mriBuffer_ReadByteAsHex(Buffer* pBuffer);
void     __mriBuffer_WriteString(Buffer* pBuffer, const char* pString);
void     __mriBuffer_WriteSizedString(Buffer* pBuffer, const char* pString, size_t length);
uint32_t __mriBuffer_ReadUIntegerAsHex(Buffer* pBuffer);
void     __mriBuffer_WriteUIntegerAsHex(Buffer* pBuffer, uint32_t value);
int32_t  __mriBuffer_ReadIntegerAsHex(Buffer* pBuffer);
void     __mriBuffer_WriteIntegerAsHex(Buffer* pBuffer, int32_t value);
int      __mriBuffer_IsNextCharEqualTo(Buffer* pBuffer, char thisChar);
int      __mriBuffer_MatchesString(Buffer* pBuffer, const char* pString, size_t stringLength);

/* Macroes which allow code to drop the __mri namespace prefix. */
#define Buffer_Init                 __mriBuffer_Init
#define Buffer_Reset                __mriBuffer_Reset
#define Buffer_SetEndOfBuffer       __mriBuffer_SetEndOfBuffer
#define Buffer_BytesLeft            __mriBuffer_BytesLeft
#define Buffer_OverrunDetected      __mriBuffer_OverrunDetected
#define Buffer_GetLength            __mriBuffer_GetLength
#define Buffer_GetArray             __mriBuffer_GetArray
#define Buffer_WriteChar            __mriBuffer_WriteChar
#define Buffer_ReadChar             __mriBuffer_ReadChar
#define Buffer_WriteByteAsHex       __mriBuffer_WriteByteAsHex
#define Buffer_ReadByteAsHex        __mriBuffer_ReadByteAsHex
#define Buffer_WriteString          __mriBuffer_WriteString
#define Buffer_WriteSizedString     __mriBuffer_WriteSizedString
#define Buffer_ReadUIntegerAsHex    __mriBuffer_ReadUIntegerAsHex
#define Buffer_WriteUIntegerAsHex   __mriBuffer_WriteUIntegerAsHex
#define Buffer_ReadIntegerAsHex     __mriBuffer_ReadIntegerAsHex
#define Buffer_WriteIntegerAsHex    __mriBuffer_WriteIntegerAsHex
#define Buffer_IsNextCharEqualTo    __mriBuffer_IsNextCharEqualTo
#define Buffer_MatchesString        __mriBuffer_MatchesString

#endif /* _BUFFER_H_ */
