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
/* Core mri functionality exposed to other modules within the debug monitor.  These are the private routines exposed
   from within mri.c.  The public functionality is exposed via mri.h. */
#ifndef _CORE_H_
#define _CORE_H_

#include <stdint.h>
#include "buffer.h"

/* Real name of functions are in __mri namespace. */
void    __mriCore_InitBuffer(void);
Buffer* __mriCore_GetBuffer(void);
Buffer* __mriCore_GetInitializedBuffer(void);
void    __mriCore_PrepareStringResponse(const char* pErrorString);
#define PrepareEmptyResponseForUnknownCommand() __mriCore_PrepareStringResponse("")

int     __mriCore_WasControlCFlagSentFromGdb(void);
void    __mriCore_RecordControlCFlagSentFromGdb(int controlCFlag);
int     __mriCore_WasSemihostCallCancelledByGdb(void);
void    __mriCore_FlagSemihostCallAsHandled(void);
int     __mriCore_IsCommShared(void);
int     __mriCore_IsFirstException(void);
int     __mriCore_WasSuccessfullyInit(void);
int     __mriCore_IsWaitingForGdbToConnect(void);

void    __mriCore_SetSignalValue(uint8_t signalValue);
uint8_t __mriCore_GetSignalValue(void);
void    __mriCore_SetSemihostReturnValues(int semihostReturnCode, int semihostErrNo);
int     __mriCore_GetSemihostReturnCode(void);
int     __mriCore_GetSemihostErrno(void);

void    __mriCore_SendPacketToGdb(void);
void    __mriCore_GdbCommandHandlingLoop(void);

/* Macroes which allow code to drop the __mri namespace prefix. */
#define InitBuffer                      __mriCore_InitBuffer
#define GetBuffer                       __mriCore_GetBuffer
#define GetInitializedBuffer            __mriCore_GetInitializedBuffer
#define PrepareStringResponse           __mriCore_PrepareStringResponse
#define WasControlCFlagSentFromGdb      __mriCore_WasControlCFlagSentFromGdb
#define RecordControlCFlagSentFromGdb   __mriCore_RecordControlCFlagSentFromGdb
#define WasSemihostCallCancelledByGdb   __mriCore_WasSemihostCallCancelledByGdb
#define FlagSemihostCallAsHandled       __mriCore_FlagSemihostCallAsHandled
#define IsCommShared                    __mriCore_IsCommShared
#define IsFirstException                __mriCore_IsFirstException
#define WasSuccessfullyInit             __mriCore_WasSuccessfullyInit
#define IsWaitingForGdbToConnect        __mriCore_IsWaitingForGdbToConnect
#define SetSignalValue                  __mriCore_SetSignalValue
#define GetSignalValue                  __mriCore_GetSignalValue
#define SetSemihostReturnValues         __mriCore_SetSemihostReturnValues
#define GetSemihostReturnCode           __mriCore_GetSemihostReturnCode
#define GetSemihostErrno                __mriCore_GetSemihostErrno
#define SendPacketToGdb                 __mriCore_SendPacketToGdb
#define GdbCommandHandlingLoop          __mriCore_GdbCommandHandlingLoop

#endif /* _CORE_H_ */
