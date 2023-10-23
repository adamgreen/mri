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
/* Declaration of routines that need to be provided for a specific target hardware platform before mri can be used to
   as a debug conduit for it. */
#ifndef PLATFORMS_H_
#define PLATFORMS_H_

#include <stdint.h>
#include <core/context.h>
#include <core/token.h>
#include <core/buffer.h>
#include <core/try_catch.h>

void      mriPlatform_Init(Token* pParameterTokens);
char*     mriPlatform_GetPacketBuffer(void);
size_t    mriPlatform_GetPacketBufferSize(void);
void      mriPlatform_EnteringDebugger(void);
void      mriPlatform_LeavingDebugger(void);

uint64_t  mriPlatform_MemRead64(uintmri_t address);
uint32_t  mriPlatform_MemRead32(uintmri_t address);
uint16_t  mriPlatform_MemRead16(uintmri_t address);
uint8_t   mriPlatform_MemRead8(uintmri_t address);
void      mriPlatform_MemWrite64(uintmri_t address, uint64_t value);
void      mriPlatform_MemWrite32(uintmri_t address, uint32_t value);
void      mriPlatform_MemWrite16(uintmri_t address, uint16_t value);
void      mriPlatform_MemWrite8(uintmri_t address, uint8_t value);
void      mriPlatform_SyncICacheToDCache(uintmri_t address, size_t size);

int       mriPlatform_CommHasReceiveData(void);
int       mriPlatform_CommHasTransmitCompleted(void);
int       mriPlatform_CommReceiveChar(void);
void      mriPlatform_CommSendBuffer(Buffer* pBuffer);
void      mriPlatform_CommSendChar(int character);

uint32_t  mriPlatform_HandleGDBCommand(Buffer* pBuffer);

typedef enum
{
    MRI_PLATFORM_TRAP_TYPE_UNKNOWN = 0,
    MRI_PLATFORM_TRAP_TYPE_HWBREAK,
    MRI_PLATFORM_TRAP_TYPE_SWBREAK,
    MRI_PLATFORM_TRAP_TYPE_WATCH,
    MRI_PLATFORM_TRAP_TYPE_RWATCH,
    MRI_PLATFORM_TRAP_TYPE_AWATCH,
} PlatformTrapType;

typedef struct
{
    PlatformTrapType    type;
    uintmri_t           address;
} PlatformTrapReason;

uint8_t             mriPlatform_DetermineCauseOfException(void);
PlatformTrapReason  mriPlatform_GetTrapReason(void);
void                mriPlatform_DisplayFaultCauseToGdbConsole(void);

void      mriPlatform_EnableSingleStep(void);
void      mriPlatform_DisableSingleStep(void);
int       mriPlatform_IsSingleStepping(void);
uintmri_t mriPlatform_GetProgramCounter(void);
void      mriPlatform_SetProgramCounter(uintmri_t newPC);
void      mriPlatform_AdvanceProgramCounterToNextInstruction(void);
int       mriPlatform_WasProgramCounterModifiedByUser(void);
int       mriPlatform_WasMemoryFaultEncountered(void);

void      mriPlatform_WriteTResponseRegistersToBuffer(Buffer* pBuffer);

size_t       mriPlatform_GetDeviceMemoryMapXmlSize(void);
const char*  mriPlatform_GetDeviceMemoryMapXml(void);
size_t       mriPlatform_GetTargetXmlSize(void);
const char*  mriPlatform_GetTargetXml(void);

typedef enum
{
    MRI_PLATFORM_WRITE_WATCHPOINT = 1,
    MRI_PLATFORM_READ_WATCHPOINT = 2,
    MRI_PLATFORM_READWRITE_WATCHPOINT = 3
}  PlatformWatchpointType;

__throws void  mriPlatform_SetHardwareBreakpointOfGdbKind(uintmri_t address, uintmri_t kind);
__throws void  mriPlatform_SetHardwareBreakpoint(uintmri_t address);
__throws void  mriPlatform_ClearHardwareBreakpointOfGdbKind(uintmri_t address, uintmri_t kind);
__throws void  mriPlatform_ClearHardwareBreakpoint(uintmri_t address);
__throws void  mriPlatform_SetHardwareWatchpoint(uintmri_t address, uintmri_t size,  PlatformWatchpointType type);
__throws void  mriPlatform_ClearHardwareWatchpoint(uintmri_t address, uintmri_t size,  PlatformWatchpointType type);

typedef enum
{
    MRI_PLATFORM_INSTRUCTION_OTHER = 0,
    MRI_PLATFORM_INSTRUCTION_MBED_SEMIHOST_CALL,
    MRI_PLATFORM_INSTRUCTION_NEWLIB_SEMIHOST_CALL,
    MRI_PLATFORM_INSTRUCTION_HARDCODED_BREAKPOINT,
}  PlatformInstructionType;

typedef struct
{
    uintmri_t    parameter1;
    uintmri_t    parameter2;
    uintmri_t    parameter3;
    uintmri_t    parameter4;
}  PlatformSemihostParameters;

PlatformInstructionType     mriPlatform_TypeOfCurrentInstruction(void);
PlatformSemihostParameters  mriPlatform_GetSemihostCallParameters(void);
void                        mriPlatform_SetSemihostCallReturnAndErrnoValues(int returnValue, int errNo);

const uint8_t*  mriPlatform_GetUid(void);
size_t          mriPlatform_GetUidSize(void);

void            mriPlatform_ResetDevice(void);

typedef enum
{
    MRI_PLATFORM_THREAD_FROZEN,
    MRI_PLATFORM_THREAD_THAWED,
    MRI_PLATFORM_THREAD_SINGLE_STEPPING
} PlatformThreadState;

/* Can be passed as threadId to Platform_RtosSetThreadState() to set state of all threads at once. */
#define MRI_PLATFORM_ALL_THREADS        ((uintmri_t)0xFFFFFFFF)
/* Can be passed as threadId to Platform_RtosSetThreadState() to set state of all threads that are still frozen. */
#define MRI_PLATFORM_ALL_FROZEN_THREADS ((uintmri_t)0xFFFFFFFE)

uintmri_t       mriPlatform_RtosGetHaltedThreadId(void);
uintmri_t       mriPlatform_RtosGetFirstThreadId(void);
uintmri_t       mriPlatform_RtosGetNextThreadId(void);
const char*     mriPlatform_RtosGetExtraThreadInfo(uintmri_t threadId);
MriContext*     mriPlatform_RtosGetThreadContext(uintmri_t threadId);
int             mriPlatform_RtosIsThreadActive(uintmri_t threadId);
int             mriPlatform_RtosIsSetThreadStateSupported(void);
void            mriPlatform_RtosSetThreadState(uintmri_t threadId, PlatformThreadState state);
void            mriPlatform_RtosRestorePrevThreadState(void);

void            mriPlatform_HandleFaultFromHighPriorityCode(void);

/* Macroes which allow code to drop the mri namespace prefix. */
#define Platform_Init                                       mriPlatform_Init
#define Platform_GetPacketBuffer                            mriPlatform_GetPacketBuffer
#define Platform_GetPacketBufferSize                        mriPlatform_GetPacketBufferSize
#define Platform_EnteringDebugger                           mriPlatform_EnteringDebugger
#define Platform_LeavingDebugger                            mriPlatform_LeavingDebugger
#define Platform_MemRead64                                  mriPlatform_MemRead64
#define Platform_MemRead32                                  mriPlatform_MemRead32
#define Platform_MemRead16                                  mriPlatform_MemRead16
#define Platform_MemRead8                                   mriPlatform_MemRead8
#define Platform_MemWrite64                                 mriPlatform_MemWrite64
#define Platform_MemWrite32                                 mriPlatform_MemWrite32
#define Platform_MemWrite16                                 mriPlatform_MemWrite16
#define Platform_MemWrite8                                  mriPlatform_MemWrite8
#define Platform_SyncICacheToDCache                         mriPlatform_SyncICacheToDCache
#define Platform_CommHasReceiveData                         mriPlatform_CommHasReceiveData
#define Platform_CommHasTransmitCompleted                   mriPlatform_CommHasTransmitCompleted
#define Platform_CommReceiveChar                            mriPlatform_CommReceiveChar
#define Platform_CommSendBuffer                             mriPlatform_CommSendBuffer
#define Platform_CommSendChar                               mriPlatform_CommSendChar
#define Platform_HandleGDBCommand                           mriPlatform_HandleGDBCommand
#define Platform_DetermineCauseOfException                  mriPlatform_DetermineCauseOfException
#define Platform_GetTrapReason                              mriPlatform_GetTrapReason
#define Platform_DisplayFaultCauseToGdbConsole              mriPlatform_DisplayFaultCauseToGdbConsole
#define Platform_EnableSingleStep                           mriPlatform_EnableSingleStep
#define Platform_DisableSingleStep                          mriPlatform_DisableSingleStep
#define Platform_IsSingleStepping                           mriPlatform_IsSingleStepping
#define Platform_GetProgramCounter                          mriPlatform_GetProgramCounter
#define Platform_SetProgramCounter                          mriPlatform_SetProgramCounter
#define Platform_AdvanceProgramCounterToNextInstruction     mriPlatform_AdvanceProgramCounterToNextInstruction
#define Platform_WasProgramCounterModifiedByUser            mriPlatform_WasProgramCounterModifiedByUser
#define Platform_WasMemoryFaultEncountered                  mriPlatform_WasMemoryFaultEncountered
#define Platform_WriteTResponseRegistersToBuffer            mriPlatform_WriteTResponseRegistersToBuffer
#define Platform_GetDeviceMemoryMapXmlSize                  mriPlatform_GetDeviceMemoryMapXmlSize
#define Platform_GetTargetXmlSize                           mriPlatform_GetTargetXmlSize
#define Platform_GetTargetXml                               mriPlatform_GetTargetXml
#define Platform_GetDeviceMemoryMapXml                      mriPlatform_GetDeviceMemoryMapXml
#define Platform_SetHardwareBreakpointOfGdbKind             mriPlatform_SetHardwareBreakpointOfGdbKind
#define Platform_SetHardwareBreakpoint                      mriPlatform_SetHardwareBreakpoint
#define Platform_ClearHardwareBreakpointOfGdbKind           mriPlatform_ClearHardwareBreakpointOfGdbKind
#define Platform_ClearHardwareBreakpoint                    mriPlatform_ClearHardwareBreakpoint
#define Platform_SetHardwareWatchpoint                      mriPlatform_SetHardwareWatchpoint
#define Platform_ClearHardwareWatchpoint                    mriPlatform_ClearHardwareWatchpoint
#define Platform_TypeOfCurrentInstruction                   mriPlatform_TypeOfCurrentInstruction
#define Platform_GetSemihostCallParameters                  mriPlatform_GetSemihostCallParameters
#define Platform_SetSemihostCallReturnAndErrnoValues        mriPlatform_SetSemihostCallReturnAndErrnoValues
#define Platform_GetUid                                     mriPlatform_GetUid
#define Platform_GetUidSize                                 mriPlatform_GetUidSize
#define Platform_ResetDevice                                mriPlatform_ResetDevice
#define Platform_RtosGetHaltedThreadId                      mriPlatform_RtosGetHaltedThreadId
#define Platform_RtosGetFirstThreadId                       mriPlatform_RtosGetFirstThreadId
#define Platform_RtosGetNextThreadId                        mriPlatform_RtosGetNextThreadId
#define Platform_RtosGetExtraThreadInfo                     mriPlatform_RtosGetExtraThreadInfo
#define Platform_RtosGetThreadContext                       mriPlatform_RtosGetThreadContext
#define Platform_RtosIsThreadActive                         mriPlatform_RtosIsThreadActive
#define Platform_RtosIsSetThreadStateSupported              mriPlatform_RtosIsSetThreadStateSupported
#define Platform_RtosSetThreadState                         mriPlatform_RtosSetThreadState
#define Platform_RtosRestorePrevThreadState                 mriPlatform_RtosRestorePrevThreadState
#define Platform_HandleFaultFromHighPriorityCode            mriPlatform_HandleFaultFromHighPriorityCode

#endif /* PLATFORMS_H_ */
