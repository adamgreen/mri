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
#include <assert.h>
#include <string.h>

extern "C"
{
#include "platforms.h"
#include "semihost.h"
#include "buffer.h"
#include "try_catch.h"
#include "hex_convert.h"
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
static size_t   getTransmitDataBufferSize();



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
static Buffer      g_receiveBuffers[2];
static int         g_receiveIndex;
static char*       g_pAlloc1;
static char*       g_pAlloc2;
static char*       g_pTransmitDataBufferStart;
static char*       g_pTransmitDataBufferEnd;
static char*       g_pTransmitDataBufferCurr;
static int         g_commInterruptBit;
static int         g_commShouldWaitForGdbConnect;
static int         g_commIsWaitingForGdbToConnect;
static int         g_commWaitForReceiveDataToStopCount;
static int         g_commPrepareToWaitForGdbConnectionCount;
static int         g_commSharingWithApplication;

void platformMock_CommInitReceiveData(const char* pDataToReceive1, const char* pDataToReceive2 /*= NULL*/)
{
    Buffer_Init(&g_receiveBuffers[0], (char*)pDataToReceive1, strlen(pDataToReceive1));
    if (pDataToReceive2)
        Buffer_Init(&g_receiveBuffers[1], (char*)pDataToReceive2, strlen(pDataToReceive2));
    else
        Buffer_Init(&g_receiveBuffers[1], (char*)g_emptyPacket, strlen(g_emptyPacket));
    g_receiveIndex = 0;
}

void platformMock_CommInitReceiveChecksummedData(const char* pDataToReceive1, const char* pDataToReceive2 /*= NULL*/)
{
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
    char checksum;
    
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
    g_pTransmitDataBufferStart = (char*)malloc(Size);
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

int platformMock_CommDoesTransmittedDataEqual(const char* thisString)
{
    size_t stringLength = strlen(thisString);

    if (getTransmitDataBufferSize() != stringLength)
        return 0;
    return !memcmp(thisString, g_pTransmitDataBufferStart, stringLength);
}

static size_t getTransmitDataBufferSize()
{
    return g_pTransmitDataBufferCurr - g_pTransmitDataBufferStart;
}

void platformMock_CommSetInterruptBit(int setValue)
{
    g_commInterruptBit = setValue;
}

void platformMock_CommSetShouldWaitForGdbConnect(int setValue)
{
    g_commShouldWaitForGdbConnect = setValue;
}

void platformMock_CommSetIsWaitingForGdbToConnectIterations(int iterations)
{
    g_commIsWaitingForGdbToConnect = iterations;
}

int platformMock_GetCommWaitForReceiveDataToStopCalls(void)
{
    return g_commWaitForReceiveDataToStopCount;
}

int platformMock_GetCommPrepareToWaitForGdbConnectionCalls(void)
{
    return g_commPrepareToWaitForGdbConnectionCount;
}

void platformMock_SetCommSharingWithApplication(int setValue)
{
    g_commSharingWithApplication = setValue;
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

int __mriPlatform_CommCausedInterrupt(void)
{
    return g_commInterruptBit;
}

void __mriPlatform_CommClearInterrupt(void)
{
    g_commInterruptBit = FALSE;
}

int __mriPlatform_CommShouldWaitForGdbConnect(void)
{
    return g_commShouldWaitForGdbConnect;
}

int __mriPlatform_CommIsWaitingForGdbToConnect(void)
{
    int returnValue = g_commIsWaitingForGdbToConnect;
    if (returnValue)
        g_commIsWaitingForGdbToConnect--;
    return returnValue;
}

void __mriPlatform_CommWaitForReceiveDataToStop(void)
{
    g_commWaitForReceiveDataToStopCount++;
}

void __mriPlatform_CommPrepareToWaitForGdbConnection(void)
{
    g_commPrepareToWaitForGdbConnectionCount++;
}

int __mriPlatform_CommSharingWithApplication(void)
{
    return g_commSharingWithApplication;
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
void __mriPlatform_EnteringDebugger(void)
{
    g_enteringDebuggerCount++;
}

void __mriPlatform_LeavingDebugger(void)
{
    g_leavingDebuggerCount++;
}



// Packet Buffer Instrumentation.
// NOTE: Packet must be big enough for g/G packets to hold 16 general purpose registers + PSR with 2 hex digits per
//       byte + 1 more byte for the g/G command character.
static char     g_packetBuffer[1 + (16 + 1) * (sizeof(uint32_t) * 2)];
static uint32_t g_packetBufferSize;

void        platformMock_SetPacketBufferSize(uint32_t setValue)
{
    assert ( setValue <= sizeof(g_packetBuffer) );
    g_packetBufferSize = setValue;
}

// Packet Buffer stubs called by MRI core.
char* __mriPlatform_GetPacketBuffer(void)
{
    return g_packetBuffer;
}

uint32_t  __mriPlatform_GetPacketBufferSize(void)
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
int __mriSemihost_IsDebuggeeMakingSemihostCall(void)
{
    return g_isDebuggeeMakingSemihostCall;
}

int __mriSemihost_HandleSemihostRequest(void)
{
    g_getHandleSemihostRequestCount++;
    return TRUE;
}



// Fault/Exception Related Instrumentation
static int g_displayFaultCauseToGdbConsoleCount;

int platformMock_DisplayFaultCauseToGdbConsoleCalls(void)
{
    return g_displayFaultCauseToGdbConsoleCount;
}

// Fault/Semihost stubs called by MRI core.
void __mriPlatform_DisplayFaultCauseToGdbConsole(void)
{
    g_displayFaultCauseToGdbConsoleCount++;
}



// Mock Setup and Cleanup APIs.
void platformMock_Init(void)
{
    platformMock_CommInitReceiveData(g_emptyPacket);
    platformMock_CommInitTransmitDataBuffer(2 * sizeof(g_packetBuffer));
    platformMock_SetInitException(noException);
    memset(&g_initTokenCopy, 0, sizeof(g_initTokenCopy));
    g_commInterruptBit = FALSE;
    g_commShouldWaitForGdbConnect = FALSE;
    g_commIsWaitingForGdbToConnect = 0;
    g_commWaitForReceiveDataToStopCount = 0;
    g_commPrepareToWaitForGdbConnectionCount = 0;
    g_commSharingWithApplication = FALSE;
    g_initCount = 0;
    g_enteringDebuggerCount = 0;
    g_leavingDebuggerCount = 0;
    g_isDebuggeeMakingSemihostCall = TRUE;
    g_getHandleSemihostRequestCount = 0;
    g_displayFaultCauseToGdbConsoleCount = 0;
    g_packetBufferSize = sizeof(g_packetBuffer);
}

void platformMock_Uninit(void)
{
    free(g_pAlloc1);
    free(g_pAlloc2);
    g_pAlloc1 = NULL;
    g_pAlloc2 = NULL;
    commUninitTransmitDataBuffer();
}



// Stubs for Platform APIs that act as NOPs when called from mriCore during testing.
uint8_t   __mriPlatform_DetermineCauseOfException(void)
{
    return SIGTRAP;
}

void      __mriPlatform_EnableSingleStep(void)
{
}

int       __mriPlatform_IsSingleStepping(void)
{
    return FALSE;
}

void      __mriPlatform_SetProgramCounter(uint32_t newPC)
{
}

void      __mriPlatform_AdvanceProgramCounterToNextInstruction(void)
{
}

int       __mriPlatform_WasProgramCounterModifiedByUser(void)
{
    return FALSE;
}

int       __mriPlatform_WasMemoryFaultEncountered(void)
{
    return FALSE;
}


void __mriPlatform_WriteTResponseRegistersToBuffer(Buffer* pBuffer)
{
    Buffer_WriteString(pBuffer, "responseT");
}

void      __mriPlatform_CopyContextToBuffer(Buffer* pBuffer)
{
}

void      __mriPlatform_CopyContextFromBuffer(Buffer* pBuffer)
{
}


uint32_t     __mriPlatform_GetDeviceMemoryMapXmlSize(void)
{
    return 0;
}

const char*  __mriPlatform_GetDeviceMemoryMapXml(void)
{
    return NULL;
}

uint32_t     __mriPlatform_GetTargetXmlSize(void)
{
    return 0;
}

const char*  __mriPlatform_GetTargetXml(void)
{
    return NULL;
}


__throws void  __mriPlatform_SetHardwareBreakpoint(uint32_t address, uint32_t kind)
{
}

__throws void  __mriPlatform_ClearHardwareBreakpoint(uint32_t address, uint32_t kind)
{
}

__throws void  __mriPlatform_SetHardwareWatchpoint(uint32_t address, uint32_t size,  PlatformWatchpointType type)
{
}

__throws void  __mriPlatform_ClearHardwareWatchpoint(uint32_t address, uint32_t size,  PlatformWatchpointType type)
{
}


PlatformInstructionType     __mriPlatform_TypeOfCurrentInstruction(void)
{
    return MRI_PLATFORM_INSTRUCTION_OTHER;
}

void                        __mriPlatform_SetSemihostCallReturnValue(uint32_t returnValue)
{
}


extern "C" void __mriPlatform_EnteringDebuggerHook(void)
{
}

extern "C" void __mriPlatform_LeavingDebuggerHook(void)
{
}
