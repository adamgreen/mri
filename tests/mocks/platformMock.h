/* Copyright 2022 Adam Green (https://github.com/adamgreen/)

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
#ifndef PLATFORM_MOCK_H_
#define PLATFORM_MOCK_H_

extern "C"
{
    #include <core/context.h>
    #include <core/token.h>
    #include <core/platforms.h>
}

#define INITIAL_PC 0x10000000

void        platformMock_Init(void);
void        platformMock_Uninit(void);

void        platformMock_CommInitReceiveData(const char* pDataToReceive1,
                                             const char* pDataToReceive2 = NULL,
                                             const char* pDataToReceive3 = NULL);
void        platformMock_CommInitReceiveChecksummedData(const char* pDataToReceive1,
                                                        const char* pDataToReceive2 = NULL,
                                                        const char* pDataToReceive3 = NULL);
void        platformMock_CommInitTransmitDataBuffer(size_t Size);
const char* platformMock_CommChecksumData(const char* pData);
const char* platformMock_CommGetTransmittedData(void);
int         platformMock_CommGetHasTransmitCompletedCallCount(void);

void        platformMock_SetInitException(int exceptionToThrow);
int         platformMock_GetInitCount(void);
Token*      platformMock_GetInitTokenCopy(void);

int         platformMock_GetEnteringDebuggerCalls(void);
int         platformMock_GetLeavingDebuggerCalls(void);

void        platformMock_SetIsDebuggeeMakingSemihostCall(int setValue);
int         platformMock_GetHandleSemihostRequestCalls(void);

void        platformMock_SetCauseOfException(uint8_t signal);
void        platformMock_SetTrapReason(const PlatformTrapReason* reason);
int         platformMock_DisplayFaultCauseToGdbConsoleCalls(void);

void        platformMock_SetPacketBufferSize(uint32_t setValue);

void        platformMock_SetTypeOfCurrentInstruction(PlatformInstructionType setValue);
int         platformMock_AdvanceProgramCounterToNextInstructionCalls(void);
int         platformMock_SetProgramCounterCalls(void);
uint32_t    platformMock_GetProgramCounterValue(void);

void        platformMock_FaultOnSpecificMemoryCall(int callToFail);

uint32_t*   platformMock_GetContextEntries(void);
MriContext* platformMock_GetContext(void);

int         platformMock_SetHardwareBreakpointCalls(void);
uint32_t    platformMock_SetHardwareBreakpointAddressArg(void);
uint32_t    platformMock_SetHardwareBreakpointKindArg(void);
void        platformMock_SetHardwareBreakpointException(uint32_t exceptionToThrow);

int         platformMock_ClearHardwareBreakpointCalls(void);
uint32_t    platformMock_ClearHardwareBreakpointAddressArg(void);
uint32_t    platformMock_ClearHardwareBreakpointKindArg(void);
void        platformMock_ClearHardwareBreakpointException(uint32_t exceptionToThrow);

int                    platformMock_SetHardwareWatchpointCalls(void);
uint32_t               platformMock_SetHardwareWatchpointAddressArg(void);
uint32_t               platformMock_SetHardwareWatchpointSizeArg(void);
PlatformWatchpointType platformMock_SetHardwareWatchpointTypeArg(void);
void                   platformMock_SetHardwareWatchpointException(uint32_t exceptionToThrow);

int                    platformMock_ClearHardwareWatchpointCalls(void);
uint32_t               platformMock_ClearHardwareWatchpointAddressArg(void);
uint32_t               platformMock_ClearHardwareWatchpointSizeArg(void);
PlatformWatchpointType platformMock_ClearHardwareWatchpointTypeArg(void);
void                   platformMock_ClearHardwareWatchpointException(uint32_t exceptionToThrow);

int platformMock_GetSemihostCallReturnValue(void);
int platformMock_GetSemihostCallErrno(void);

int platformMock_GetResetDeviceCalls(void);

struct PlatformMockThread
{
    uint32_t            threadId;
    PlatformThreadState state;
};

void platformMock_SetSingleStepState(int state);
void platformMock_SingleStepShouldAdvancePC(bool enable);

void platformMock_RtosSetHaltedThreadId(uint32_t threadId);
void platformMock_RtosSetThreads(const uint32_t* pThreadArray, uint32_t threadCount);
void platformMock_RtosSetExtraThreadInfo(uint32_t threadId, const char* pExtraThreadInfo);
void platformMock_RtosSetThreadContext(uint32_t threadId, MriContext* pContext);
void platformMock_RtosSetActiveThread(uint32_t threadId);
void platformMock_RtosSetIsSetThreadStateSupported(int isSupported);
void     platformMock_RtosSetThreadList(PlatformMockThread* pThreads, size_t threadCount);
uint32_t platformMock_RtosGetThreadStateInvalidAttempts(void);
uint32_t platformMock_RtosGetRestorePrevThreadStateCallCount(void);

int platformMock_GetInvalidateICacheCalls(void);

#endif /* PLATFORM_MOCK_H_ */
