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
#include <assert.h>
#include <string.h>

extern "C"
{
#include <core/platforms.h>
#include <core/signal.h>
#include <core/semihost.h>
#include <core/buffer.h>
#include <core/try_catch.h>
#include <core/hex_convert.h>
#include <core/memory.h>
}
#include "platformMock.h"

#define TRUE 1
#define FALSE 0
#define ARRAY_SIZE(X) (sizeof(X)/sizeof(X[0]))


// Forward Function Declarations.
static char*    allocateAndCopyChecksummedData(const char* pData);
static size_t   countPoundSigns(const char* p);
static void     copyChecksummedData(char* pDest, const char* pSrc);
static void     commUninitTransmitDataBuffer();
static uint32_t isReceiveBufferEmpty();
static void     waitForReceiveData();
static void     skipNullThreadIds();



// Platform_Init Instrumentation
static int   g_initException;
static int   g_initCount;
static Token g_initTokenCopy;

void platformMock_SetInitException(int exceptionToThrow)
{
    g_initException = exceptionToThrow;
}

int platformMock_GetInitCount()
{
    return g_initCount;
}

Token* platformMock_GetInitTokenCopy()
{
    return &g_initTokenCopy;
}

// Platform_Init stub called by MRI core.
void Platform_Init(Token* pParameterTokens)
{
    g_initCount++;
    Token_Copy(&g_initTokenCopy, pParameterTokens);

    if (g_initException)
        __throw(g_initException);
}



// Platform_Comm* Instrumentation
static const char  g_emptyPacket[] = "$#00";
static Buffer      g_receiveBuffers[3];
static size_t      g_receiveIndex;
static char*       g_pAlloc1;
static char*       g_pAlloc2;
static char*       g_pAlloc3;
static char*       g_pTransmitDataBufferStart;
static char*       g_pTransmitDataBufferEnd;
static char*       g_pTransmitDataBufferCurr;
static char*       g_pChecksumData;
static int         g_hasTransmitCompletedCount;

void platformMock_CommInitReceiveData(const char* pDataToReceive1,
                                      const char* pDataToReceive2 /* = NULL */,
                                      const char* pDataToReceive3 /* = NULL */)
{
    Buffer_Init(&g_receiveBuffers[0], (char*)pDataToReceive1, strlen(pDataToReceive1));
    if (pDataToReceive2)
        Buffer_Init(&g_receiveBuffers[1], (char*)pDataToReceive2, strlen(pDataToReceive2));
    else
        Buffer_Init(&g_receiveBuffers[1], (char*)g_emptyPacket, strlen(g_emptyPacket));
    if (pDataToReceive3)
        Buffer_Init(&g_receiveBuffers[2], (char*)pDataToReceive3, strlen(pDataToReceive3));
    else
        Buffer_Init(&g_receiveBuffers[2], (char*)g_emptyPacket, strlen(g_emptyPacket));
    g_receiveIndex = 0;
}

void platformMock_CommInitReceiveChecksummedData(const char* pDataToReceive1,
                                                 const char* pDataToReceive2 /* = NULL */,
                                                 const char* pDataToReceive3 /* = NULL */)
{
    free(g_pAlloc1);
    free(g_pAlloc2);
    free(g_pAlloc3);
    g_pAlloc1 = NULL;
    g_pAlloc2 = NULL;
    g_pAlloc3 = NULL;

    g_pAlloc1 = allocateAndCopyChecksummedData(pDataToReceive1);
    Buffer_Init(&g_receiveBuffers[0], g_pAlloc1, strlen(g_pAlloc1));
    if (pDataToReceive2)
    {
        g_pAlloc2 = allocateAndCopyChecksummedData(pDataToReceive2);
        Buffer_Init(&g_receiveBuffers[1], g_pAlloc2, strlen(g_pAlloc2));
    }
    else
    {
        Buffer_Init(&g_receiveBuffers[1], (char*)g_emptyPacket, strlen(g_emptyPacket));
    }
    if (pDataToReceive3)
    {
        g_pAlloc3 = allocateAndCopyChecksummedData(pDataToReceive3);
        Buffer_Init(&g_receiveBuffers[2], g_pAlloc3, strlen(g_pAlloc3));
    }
    else
    {
        Buffer_Init(&g_receiveBuffers[2], (char*)g_emptyPacket, strlen(g_emptyPacket));
    }
    g_receiveIndex = 0;
}

static char* allocateAndCopyChecksummedData(const char* pData)
{
    size_t len = strlen(pData) + 2 * countPoundSigns(pData) + 1;
    char*  pAlloc = (char*) malloc(len);
    copyChecksummedData(pAlloc, pData);
    return pAlloc;
}

static size_t countPoundSigns(const char* p)
{
    size_t count = 0;
    while (*p)
    {
        if (*p++ == '#')
            count++;
    }
    return count;
}

static void copyChecksummedData(char* pDest, const char* pSrc)
{
    char checksum = 0;

    while (*pSrc)
    {
        char curr = *pSrc++;

        *pDest++ = curr;
        switch (curr)
        {
        case '$':
            checksum = 0;
            break;
        case '#':
            *pDest++ = NibbleToHexChar[EXTRACT_HI_NIBBLE(checksum)];
            *pDest++ = NibbleToHexChar[EXTRACT_LO_NIBBLE(checksum)];
            break;
        default:
            checksum += curr;
            break;
        }
    }
    *pDest++ = '\0';
}

void platformMock_CommInitTransmitDataBuffer(size_t Size)
{
    commUninitTransmitDataBuffer();
    g_pTransmitDataBufferStart = (char*)malloc(Size+1);
    g_pTransmitDataBufferCurr = g_pTransmitDataBufferStart;
    g_pTransmitDataBufferEnd = g_pTransmitDataBufferStart + Size;
}

static void commUninitTransmitDataBuffer()
{
    free(g_pTransmitDataBufferStart);
    g_pTransmitDataBufferStart = NULL;
    g_pTransmitDataBufferCurr = NULL;
    g_pTransmitDataBufferEnd = NULL;
}

const char* platformMock_CommChecksumData(const char* pData)
{
    free(g_pChecksumData);
    g_pChecksumData = allocateAndCopyChecksummedData(pData);
    return g_pChecksumData;
}

const char* platformMock_CommGetTransmittedData(void)
{
    // Room was always reserved for NULL terminator at init time.
    *g_pTransmitDataBufferCurr = '\0';
    return g_pTransmitDataBufferStart;
}

int platformMock_CommGetHasTransmitCompletedCallCount(void)
{
    return g_hasTransmitCompletedCount;
}

// Platform_Comm* stubs called by MRI core.
uint32_t Platform_CommHasReceiveData(void)
{
    if (isReceiveBufferEmpty())
    {
        if (g_receiveIndex < ARRAY_SIZE(g_receiveBuffers))
            g_receiveIndex++;
        return 0;
    }

    return 1;
}

static uint32_t isReceiveBufferEmpty()
{
    if (g_receiveIndex >= ARRAY_SIZE(g_receiveBuffers))
        return TRUE;
    return (uint32_t)(Buffer_BytesLeft(&g_receiveBuffers[g_receiveIndex]) == 0);
}

uint32_t  Platform_CommHasTransmitCompleted(void)
{
    g_hasTransmitCompletedCount++;
    return TRUE;
}

int Platform_CommReceiveChar(void)
{
    waitForReceiveData();

    int character = Buffer_ReadChar(&g_receiveBuffers[g_receiveIndex]);

    clearExceptionCode();

    return character;
}

static void waitForReceiveData()
{
    while (!Platform_CommHasReceiveData())
    {
    }
}

void Platform_CommSendChar(int character)
{
    if (g_pTransmitDataBufferCurr < g_pTransmitDataBufferEnd)
        *g_pTransmitDataBufferCurr++ = (char)character;
}


// Instrumentation to entering and leaving of debugger.
static int g_enteringDebuggerCount;
static int g_leavingDebuggerCount;

int platformMock_GetEnteringDebuggerCalls(void)
{
    return g_enteringDebuggerCount;
}

int platformMock_GetLeavingDebuggerCalls(void)
{
    return g_leavingDebuggerCount;
}

// Stubs called by MRI core.
void Platform_EnteringDebugger(void)
{
    g_enteringDebuggerCount++;
}

void Platform_LeavingDebugger(void)
{
    g_leavingDebuggerCount++;
}



// Packet Buffer Instrumentation.
// NOTE: Packet must be big enough for g/G packets to hold 16 general purpose registers + PSR with 2 hex digits per
//       byte + 1 more byte for the g/G command character + 4 more bytes for packet header/checksum_trailer.
static char     g_packetBuffer[1 + (16 + 1) * (sizeof(uint32_t) * 2) + 4];
static uint32_t g_packetBufferSize;

void platformMock_SetPacketBufferSize(uint32_t setValue)
{
    assert ( setValue <= sizeof(g_packetBuffer) );
    g_packetBufferSize = setValue;
}

// Packet Buffer stubs called by MRI core.
char* Platform_GetPacketBuffer(void)
{
    return g_packetBuffer;
}

uint32_t  Platform_GetPacketBufferSize(void)
{
    return g_packetBufferSize;
}



// Semihost Instrumentation
static int g_isDebuggeeMakingSemihostCall;
static int g_getHandleSemihostRequestCount;

void platformMock_SetIsDebuggeeMakingSemihostCall(int setValue)
{
    g_isDebuggeeMakingSemihostCall = setValue;
}

int platformMock_GetHandleSemihostRequestCalls(void)
{
    return g_getHandleSemihostRequestCount;
}

// Semihost stubs called by MRI core.
int Semihost_IsDebuggeeMakingSemihostCall(void)
{
    return g_isDebuggeeMakingSemihostCall;
}

int Semihost_HandleSemihostRequest(void)
{
    g_getHandleSemihostRequestCount++;
    return TRUE;
}



// Fault/Exception Related Instrumentation
static uint8_t            g_causeOfException;
static int                g_displayFaultCauseToGdbConsoleCount;
static PlatformTrapReason g_trapReason;

void platformMock_SetCauseOfException(uint8_t signal)
{
    g_causeOfException = signal;
}

int platformMock_DisplayFaultCauseToGdbConsoleCalls(void)
{
    return g_displayFaultCauseToGdbConsoleCount;
}

void platformMock_SetTrapReason(const PlatformTrapReason* pReason)
{
    g_trapReason = *pReason;
}


// Fault/Exception stubs called by MRI core.
uint8_t Platform_DetermineCauseOfException(void)
{
    return g_causeOfException;
}

PlatformTrapReason Platform_GetTrapReason(void)
{
    return g_trapReason;
}

void Platform_DisplayFaultCauseToGdbConsole(void)
{
    g_displayFaultCauseToGdbConsoleCount++;
}



// Current Instruction Related Instrumentation
PlatformInstructionType g_instructionType;
int                     g_advanceProgramCounterToNextInstruction;
int                     g_setProgramCounterCalls;
uint32_t                g_programCounter;

void platformMock_SetTypeOfCurrentInstruction(PlatformInstructionType setValue)
{
    g_instructionType = setValue;
}

int platformMock_AdvanceProgramCounterToNextInstructionCalls(void)
{
    return g_advanceProgramCounterToNextInstruction;
}

int platformMock_SetProgramCounterCalls(void)
{
    return g_setProgramCounterCalls;
}

uint32_t platformMock_GetProgramCounterValue(void)
{
    return g_programCounter;
}

// Stubs called by MRI core.
PlatformInstructionType Platform_TypeOfCurrentInstruction(void)
{
    return g_instructionType;
}

void Platform_AdvanceProgramCounterToNextInstruction(void)
{
    g_advanceProgramCounterToNextInstruction++;
    g_programCounter += 4;
}

void Platform_SetProgramCounter(uint32_t newPC)
{
    g_programCounter = newPC;
    g_setProgramCounterCalls++;
}

int Platform_WasProgramCounterModifiedByUser(void)
{
    return FALSE;
}

uint32_t  Platform_GetProgramCounter(void)
{
    return g_programCounter;
}



// Single Stepping stubs called by MRI core.
static int g_singleStepping;
static bool g_singleSteppingForced = false;
static bool g_singleSteppingShouldAdvancePC = false;

void platformMock_SetSingleStepState(int state)
{
    g_singleSteppingForced = true;
    g_singleStepping = state;
}

void platformMock_SingleStepShouldAdvancePC(bool enable)
{
    g_singleSteppingShouldAdvancePC = enable;
}

void Platform_EnableSingleStep(void)
{
    if (g_singleSteppingShouldAdvancePC)
        g_programCounter += 4;
    if (!g_singleSteppingForced)
        g_singleStepping = TRUE;
}

void Platform_DisableSingleStep(void)
{
    if (!g_singleSteppingForced)
        g_singleStepping = FALSE;
}

int Platform_IsSingleStepping(void)
{
    return g_singleStepping;
}


// Memory Fault Test Instrumentation.
static int g_callToFail;

void platformMock_FaultOnSpecificMemoryCall(int callToFail)
{
    g_callToFail = callToFail;
}

// Stub called by MRI core.
int Platform_WasMemoryFaultEncountered(void)
{
    if (g_callToFail == 0)
        return FALSE;
    g_callToFail--;
    if (g_callToFail == 0)
        return TRUE;
    return FALSE;
}



// Context Related Instrumentation.
static uint32_t         g_contextEntries[4];
static ContextSection   g_contextSection = { .pValues = g_contextEntries, .count = 4 };
static MriContext       g_context;

uint32_t* platformMock_GetContextEntries(void)
{
    return g_contextEntries;
}

MriContext* platformMock_GetContext(void)
{
    return &g_context;
}

// Stubs called from MRI core.
void Platform_WriteTResponseRegistersToBuffer(Buffer* pBuffer)
{
    Buffer_WriteString(pBuffer, "responseT");
}



// Hardware Breakpoint / Watchpoint Instrumentation.
int                    g_setHardwareBreakpointCalls;
uint32_t               g_setHardwareBreakpointAddressArg;
uint32_t               g_setHardwareBreakpointKindArg;
uint32_t               g_setHardwareBreakpointException;
int                    g_clearHardwareBreakpointCalls;
uint32_t               g_clearHardwareBreakpointAddressArg;
uint32_t               g_clearHardwareBreakpointKindArg;
uint32_t               g_clearHardwareBreakpointException;
int                    g_setHardwareWatchpointCalls;
uint32_t               g_setHardwareWatchpointAddressArg;
uint32_t               g_setHardwareWatchpointSizeArg;
PlatformWatchpointType g_setHardwareWatchpointTypeArg;
uint32_t               g_setHardwareWatchpointException;
int                    g_clearHardwareWatchpointCalls;
uint32_t               g_clearHardwareWatchpointAddressArg;
uint32_t               g_clearHardwareWatchpointSizeArg;
PlatformWatchpointType g_clearHardwareWatchpointTypeArg;
uint32_t               g_clearHardwareWatchpointException;

int platformMock_SetHardwareBreakpointCalls(void)
{
    return g_setHardwareBreakpointCalls;
}

uint32_t platformMock_SetHardwareBreakpointAddressArg(void)
{
    return g_setHardwareBreakpointAddressArg;
}

uint32_t platformMock_SetHardwareBreakpointKindArg(void)
{
    return g_setHardwareBreakpointKindArg;
}

void platformMock_SetHardwareBreakpointException(uint32_t exceptionToThrow)
{
    g_setHardwareBreakpointException = exceptionToThrow;
}

int platformMock_ClearHardwareBreakpointCalls(void)
{
    return g_clearHardwareBreakpointCalls;
}

uint32_t platformMock_ClearHardwareBreakpointAddressArg(void)
{
    return g_clearHardwareBreakpointAddressArg;
}

uint32_t platformMock_ClearHardwareBreakpointKindArg(void)
{
    return g_clearHardwareBreakpointKindArg;
}

void platformMock_ClearHardwareBreakpointException(uint32_t exceptionToThrow)
{
    g_clearHardwareBreakpointException = exceptionToThrow;
}

int platformMock_SetHardwareWatchpointCalls(void)
{
    return g_setHardwareWatchpointCalls;
}

uint32_t platformMock_SetHardwareWatchpointAddressArg(void)
{
    return g_setHardwareWatchpointAddressArg;
}

uint32_t platformMock_SetHardwareWatchpointSizeArg(void)
{
    return g_setHardwareWatchpointSizeArg;
}

PlatformWatchpointType platformMock_SetHardwareWatchpointTypeArg(void)
{
    return g_setHardwareWatchpointTypeArg;
}

void platformMock_SetHardwareWatchpointException(uint32_t exceptionToThrow)
{
    g_setHardwareWatchpointException = exceptionToThrow;
}

int platformMock_ClearHardwareWatchpointCalls(void)
{
    return g_clearHardwareWatchpointCalls;
}

uint32_t platformMock_ClearHardwareWatchpointAddressArg(void)
{
    return g_clearHardwareWatchpointAddressArg;
}

uint32_t platformMock_ClearHardwareWatchpointSizeArg(void)
{
    return g_clearHardwareWatchpointSizeArg;
}

PlatformWatchpointType platformMock_ClearHardwareWatchpointTypeArg(void)
{
    return g_clearHardwareWatchpointTypeArg;
}

void platformMock_ClearHardwareWatchpointException(uint32_t exceptionToThrow)
{
    g_clearHardwareWatchpointException = exceptionToThrow;
}

// Stubs called from MRI core.
__throws void  Platform_SetHardwareBreakpointOfGdbKind(uint32_t address, uint32_t kind)
{
    g_setHardwareBreakpointCalls++;
    g_setHardwareBreakpointAddressArg = address;
    g_setHardwareBreakpointKindArg = kind;
    if (g_setHardwareBreakpointException)
        __throw(g_setHardwareBreakpointException);
}

__throws void  Platform_SetHardwareBreakpoint(uint32_t address)
{
    g_setHardwareBreakpointCalls++;
    g_setHardwareBreakpointAddressArg = address;
    g_setHardwareBreakpointKindArg = 0xFFFFFFFF;
    if (g_setHardwareBreakpointException)
        __throw(g_setHardwareBreakpointException);
}

__throws void  Platform_ClearHardwareBreakpointOfGdbKind(uint32_t address, uint32_t kind)
{
    g_clearHardwareBreakpointCalls++;
    g_clearHardwareBreakpointAddressArg = address;
    g_clearHardwareBreakpointKindArg = kind;
    if (g_clearHardwareBreakpointException)
        __throw(g_clearHardwareBreakpointException);
}

__throws void  Platform_ClearHardwareBreakpoint(uint32_t address)
{
    g_clearHardwareBreakpointCalls++;
    g_clearHardwareBreakpointAddressArg = address;
    g_clearHardwareBreakpointKindArg = 0xFFFFFFFF;
    if (g_clearHardwareBreakpointException)
        __throw(g_clearHardwareBreakpointException);
}

__throws void  Platform_SetHardwareWatchpoint(uint32_t address, uint32_t size,  PlatformWatchpointType type)
{
    g_setHardwareWatchpointCalls++;
    g_setHardwareWatchpointAddressArg = address;
    g_setHardwareWatchpointSizeArg = size;
    g_setHardwareWatchpointTypeArg = type;
    if (g_setHardwareWatchpointException)
        __throw(g_setHardwareWatchpointException);
}

__throws void  Platform_ClearHardwareWatchpoint(uint32_t address, uint32_t size,  PlatformWatchpointType type)
{
    g_clearHardwareWatchpointCalls++;
    g_clearHardwareWatchpointAddressArg = address;
    g_clearHardwareWatchpointSizeArg = size;
    g_clearHardwareWatchpointTypeArg = type;
    if (g_clearHardwareWatchpointException)
        __throw(g_clearHardwareWatchpointException);
}



// Query memory map and feature XML test instrumentation.
static char g_deviceMemoryMapXml[] = "TEST";
static char g_targetXml[] = "test!";

// Stubs called by MRI core.
uint32_t Platform_GetDeviceMemoryMapXmlSize(void)
{
    return sizeof(g_deviceMemoryMapXml) - 1;
}

const char*  Platform_GetDeviceMemoryMapXml(void)
{
    return g_deviceMemoryMapXml;
}

uint32_t Platform_GetTargetXmlSize(void)
{
    return sizeof(g_targetXml) - 1;
}

const char*  Platform_GetTargetXml(void)
{
    return g_targetXml;
}



// Semihost Test Instrumentation.
static int g_semihostCallReturnValue;
static int g_semihostCallErrno;

int platformMock_GetSemihostCallReturnValue(void)
{
    return g_semihostCallReturnValue;
}

int platformMock_GetSemihostCallErrno(void)
{
    return g_semihostCallErrno;
}

// Stubs called by MRI core.
void Platform_SetSemihostCallReturnAndErrnoValues(int returnValue, int err)
{
    g_semihostCallReturnValue = returnValue;
    g_semihostCallErrno = err;
}



// Device Reset Instrumentation.
static int g_resetCount;
int platformMock_GetResetDeviceCalls(void)
{
    return g_resetCount;
}

// Stubs called by MRI core.
void Platform_ResetDevice(void)
{
    g_resetCount++;
}



// RTOS Thread related instrumentation.
static uint32_t g_rtosThreadId;
static uint32_t g_rtosThreadIndex;
static uint32_t g_rtosThreadCount;
static const uint32_t* g_pRtosThreads;
static uint32_t    g_rtosExtraThreadInfoThreadId;
static const char* g_pRtosExtraThreadInfo;
static uint32_t    g_rtosContextThreadId;
static MriContext* g_pRtosContext;
static uint32_t    g_rtosActiveThread;
static int         g_isRtosSetThreadStateSupported;
static PlatformMockThread* g_pRtosThreadStates;
static size_t      g_rtosThreadStateCount;
static uint32_t    g_rtosInvalidThreadAttempts;
static uint32_t    g_rtosRestorePrevThreadStateCallCount;

void platformMock_RtosSetHaltedThreadId(uint32_t threadId)
{
    g_rtosThreadId = threadId;
}

void platformMock_RtosSetThreads(const uint32_t* pThreadArray, uint32_t threadCount)
{
    g_rtosThreadCount = threadCount;
    g_pRtosThreads = pThreadArray;
}

void platformMock_RtosSetExtraThreadInfo(uint32_t threadId, const char* pExtraThreadInfo)
{
    g_rtosExtraThreadInfoThreadId = threadId;
    g_pRtosExtraThreadInfo = pExtraThreadInfo;
}

void platformMock_RtosSetThreadContext(uint32_t threadId, MriContext* pContext)
{
    g_rtosContextThreadId = threadId;
    g_pRtosContext = pContext;
}

void platformMock_RtosSetActiveThread(uint32_t threadId)
{
    g_rtosActiveThread = threadId;
}

void platformMock_RtosSetIsSetThreadStateSupported(int isSupported)
{
    g_isRtosSetThreadStateSupported = isSupported;
}

void platformMock_RtosSetThreadList(PlatformMockThread* pThreads, size_t threadCount)
{
    g_pRtosThreadStates = pThreads;
    g_rtosThreadStateCount = threadCount;
}

uint32_t platformMock_RtosGetThreadStateInvalidAttempts(void)
{
    return g_rtosInvalidThreadAttempts;
}

uint32_t platformMock_RtosGetRestorePrevThreadStateCallCount(void)
{
    return g_rtosRestorePrevThreadStateCallCount;
}

// Stubs called by MRI core.
uint32_t Platform_RtosGetHaltedThreadId(void)
{
    return g_rtosThreadId;
}

uint32_t Platform_RtosGetFirstThreadId(void)
{
    g_rtosThreadIndex = 0;
    return Platform_RtosGetNextThreadId();
}

uint32_t Platform_RtosGetNextThreadId(void)
{
    skipNullThreadIds();
    if (g_rtosThreadIndex >= g_rtosThreadCount)
        return 0;
    return g_pRtosThreads[g_rtosThreadIndex++];
}

static void skipNullThreadIds()
{
    while (g_rtosThreadIndex < g_rtosThreadCount && g_pRtosThreads[g_rtosThreadIndex] == 0)
        g_rtosThreadIndex++;
}

const char* Platform_RtosGetExtraThreadInfo(uint32_t threadID)
{
    if (threadID == g_rtosExtraThreadInfoThreadId)
        return g_pRtosExtraThreadInfo;
    else
        return NULL;
}

MriContext* Platform_RtosGetThreadContext(uint32_t threadId)
{
    if (g_rtosContextThreadId == threadId)
        return g_pRtosContext;
    return NULL;
}

int Platform_RtosIsThreadActive(uint32_t threadId)
{
    return threadId == g_rtosActiveThread;
}

int Platform_RtosIsSetThreadStateSupported(void)
{
    return g_isRtosSetThreadStateSupported;
}

void Platform_RtosSetThreadState(uint32_t threadId, PlatformThreadState state)
{
    bool foundThread = false;

    for (size_t i = 0 ; i < g_rtosThreadStateCount ; i++)
    {
        if (g_pRtosThreadStates[i].threadId == threadId ||
            threadId == MRI_PLATFORM_ALL_THREADS ||
            (threadId == MRI_PLATFORM_ALL_FROZEN_THREADS && g_pRtosThreadStates[i].state == MRI_PLATFORM_THREAD_FROZEN))
        {
            foundThread = true;
            g_pRtosThreadStates[i].state = state;
        }
    }
    if (!foundThread)
        g_rtosInvalidThreadAttempts++;
}

void Platform_RtosRestorePrevThreadState(void)
{
    g_rtosRestorePrevThreadStateCallCount++;
}



// Memory Cache instrumentation.
static int g_invalidateICacheCount;
int platformMock_GetInvalidateICacheCalls(void)
{
    return g_invalidateICacheCount;
}

void Platform_InvalidateICache(void *pv, uint32_t size)
{
    g_invalidateICacheCount++;
}





// Mock Setup and Cleanup APIs.
void platformMock_Init(void)
{
    platformMock_CommInitReceiveData(g_emptyPacket);
    platformMock_CommInitTransmitDataBuffer(2 * sizeof(g_packetBuffer));
    platformMock_SetInitException(noException);
    memset(&g_initTokenCopy, 0, sizeof(g_initTokenCopy));
    memset(&g_trapReason, 0, sizeof(g_trapReason));
    g_hasTransmitCompletedCount = 0;
    g_pChecksumData = NULL;
    g_initCount = 0;
    g_enteringDebuggerCount = 0;
    g_leavingDebuggerCount = 0;
    g_isDebuggeeMakingSemihostCall = FALSE;
    g_getHandleSemihostRequestCount = 0;
    g_causeOfException = SIGTRAP;
    g_displayFaultCauseToGdbConsoleCount = 0;
    g_packetBufferSize = sizeof(g_packetBuffer);
    g_instructionType = MRI_PLATFORM_INSTRUCTION_OTHER;
    g_advanceProgramCounterToNextInstruction = 0;
    g_setProgramCounterCalls = 0;
    g_programCounter = INITIAL_PC;
    g_singleSteppingForced = false;
    g_singleSteppingShouldAdvancePC = false;
    g_singleStepping = FALSE;
    g_callToFail = 0;
    memset(g_contextEntries, 0xff, sizeof(g_contextEntries));
    Context_Init(&g_context, &g_contextSection, 1);
    g_setHardwareBreakpointCalls = 0;
    g_setHardwareBreakpointAddressArg = 0;
    g_setHardwareBreakpointKindArg = 0;
    g_setHardwareBreakpointException = noException;
    g_clearHardwareBreakpointCalls = 0;
    g_clearHardwareBreakpointAddressArg = 0;
    g_clearHardwareBreakpointKindArg = 0;
    g_clearHardwareBreakpointException = noException;
    g_setHardwareWatchpointCalls = 0;
    g_setHardwareWatchpointAddressArg = 0;
    g_setHardwareWatchpointSizeArg = 0;
    g_setHardwareWatchpointTypeArg = MRI_PLATFORM_WRITE_WATCHPOINT;
    g_setHardwareWatchpointException = noException;
    g_clearHardwareWatchpointCalls = 0;
    g_clearHardwareWatchpointAddressArg = 0;
    g_clearHardwareWatchpointSizeArg = 0;
    g_clearHardwareWatchpointTypeArg = MRI_PLATFORM_WRITE_WATCHPOINT;
    g_clearHardwareWatchpointException = noException;
    g_semihostCallReturnValue = 0;
    g_resetCount = 0;
    g_rtosThreadId = 0;
    g_rtosThreadCount = 0;
    g_pRtosThreads = NULL;
    g_rtosExtraThreadInfoThreadId = 0;
    g_pRtosExtraThreadInfo = NULL;
    g_rtosContextThreadId = 0;
    g_pRtosContext = NULL;
    g_rtosActiveThread = 0;
    g_isRtosSetThreadStateSupported = 0;
    g_pRtosThreadStates = NULL;
    g_rtosThreadStateCount = 0;
    g_rtosInvalidThreadAttempts = 0;
    g_rtosRestorePrevThreadStateCallCount = 0;
    g_invalidateICacheCount = 0;
}

void platformMock_Uninit(void)
{
    free(g_pAlloc1);
    free(g_pAlloc2);
    free(g_pAlloc3);
    free(g_pChecksumData);
    g_pAlloc1 = NULL;
    g_pAlloc2 = NULL;
    g_pAlloc3 = NULL;
    g_pChecksumData = NULL;
    commUninitTransmitDataBuffer();
}



// Stubs for Platform APIs that act as NOPs when called from mriCore during testing.
extern "C" uint32_t  Platform_HandleGDBCommand(Buffer* pBuffer)
{
    return 0;
}
