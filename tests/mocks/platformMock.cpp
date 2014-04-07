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
#include <string.h>

extern "C"
{
#include "platforms.h"
#include "semihost.h"
#include "buffer.h"
#include "try_catch.h"
}
#include "platformMock.h"

#define TRUE 1
#define FALSE 0


// Forward Function Declarations.
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

void platformMock_ClearInitCount()
{
    g_initCount = 0;
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
static Buffer      g_receiveBuffer;
static char*       g_pTransmitDataBufferStart;
static char*       g_pTransmitDataBufferEnd;
static char*       g_pTransmitDataBufferCurr;
void platformMock_CommInitReceiveData(const char* pDataToReceive)
{
    Buffer_Init(&g_receiveBuffer, (char*)pDataToReceive, strlen(pDataToReceive));
}

void platformMock_CommInitTransmitDataBuffer(size_t Size)
{
    platformMock_CommUninitTransmitDataBuffer();
    g_pTransmitDataBufferStart = (char*)malloc(Size);
    g_pTransmitDataBufferCurr = g_pTransmitDataBufferStart;
    g_pTransmitDataBufferEnd = g_pTransmitDataBufferStart + Size;
}

void platformMock_CommUninitTransmitDataBuffer()
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

// Platform_Comm* stubs called by MRI core.
uint32_t Platform_CommHasReceiveData(void)
{
    if (isReceiveBufferEmpty())
    {
        Buffer_Init(&g_receiveBuffer, (char*)g_emptyPacket, strlen(g_emptyPacket));
        return 0;
    }
    
    return 1;
}

static uint32_t isReceiveBufferEmpty()
{
    return (uint32_t)(Buffer_BytesLeft(&g_receiveBuffer) == 0);
}

int Platform_CommReceiveChar(void)
{
    waitForReceiveData();

    int character = Buffer_ReadChar(&g_receiveBuffer);

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



// Mock Setup and Cleanup APIs.
void platformMock_Init(void)
{
    platformMock_CommInitReceiveData(g_emptyPacket);
    platformMock_SetInitException(noException);
    platformMock_ClearInitCount();
    memset(&g_initTokenCopy, 0, sizeof(g_initTokenCopy));
}

void platformMock_Uninit(void)
{
    platformMock_CommUninitTransmitDataBuffer();
}



// Stubs for Platform APIs that act as NOPs when called from mriCore during testing.
char*     __mriPlatform_GetPacketBuffer(void)
{
    return NULL;
}

uint32_t  __mriPlatform_GetPacketBufferSize(void)
{
    return 0;
}

void      __mriPlatform_EnteringDebugger(void)
{
}

void      __mriPlatform_LeavingDebugger(void)
{
}


int       __mriPlatform_CommCausedInterrupt(void)
{
    return FALSE;
}

void      __mriPlatform_CommClearInterrupt(void)
{
}

int       __mriPlatform_CommShouldWaitForGdbConnect(void)
{
    return FALSE;
}

void      __mriPlatform_CommPrepareToWaitForGdbConnection(void)
{
}

int       __mriPlatform_CommIsWaitingForGdbToConnect(void)
{
    return FALSE;
}

void      __mriPlatform_CommWaitForReceiveDataToStop(void)
{
}


uint8_t   __mriPlatform_DetermineCauseOfException(void)
{
    return 0;
}

void      __mriPlatform_DisplayFaultCauseToGdbConsole(void)
{
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


void      __mriPlatform_WriteTResponseRegistersToBuffer(Buffer* pBuffer)
{
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


int __mriSemihost_IsDebuggeeMakingSemihostCall(void)
{
    return 0;
}

int __mriSemihost_HandleSemihostRequest(void)
{
    return 0;
}


extern "C" void __mriPlatform_EnteringDebuggerHook(void)
{
}

extern "C" void __mriPlatform_LeavingDebuggerHook(void)
{
}
