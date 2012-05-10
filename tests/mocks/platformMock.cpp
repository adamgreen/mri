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

extern "C"
{
#include "../../mri/platforms/devices/lpc176x/lpc176x_uart.h"
#include "platforms.h"
#include "semihost.h"
#include "buffer.h"
#include "try_catch.h"
}
#include "platformMock.h"

static const char  g_emptyPacket[] = "$#00";
static Buffer      g_receiveBuffer;
static char*       g_pTransmitDataBufferStart;
static char*       g_pTransmitDataBufferEnd;
static char*       g_pTransmitDataBufferCurr;
static int         g_commShareFlag;

static int         g_initException;
static int         g_initCount;
static Token       g_initTokenCopy;

static int         g_disableSingleStepCount;


static uint32_t isReceiveBufferEmpty();
static void     waitForReceiveData();
static size_t   getTransmitDataBufferSize();


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

uint32_t Platform_CommHasReceiveData(void)
{
    if (isReceiveBufferEmpty())
    {
        Buffer_Init(&g_receiveBuffer, (char*)g_emptyPacket, strlen(g_emptyPacket));
        return 0;
    }
    
    return 1;
}

int Platform_CommReceiveChar(void)
{
    waitForReceiveData();

    int character = Buffer_ReadChar(&g_receiveBuffer);

    clearExceptionCode();

    return character;
}

void Platform_CommSendChar(int character)
{
    if (g_pTransmitDataBufferCurr < g_pTransmitDataBufferEnd)
        *g_pTransmitDataBufferCurr++ = (char)character;
}

static uint32_t isReceiveBufferEmpty()
{
    return (uint32_t)(Buffer_BytesLeft(&g_receiveBuffer) == 0);
}

static void waitForReceiveData()
{
    while (!Platform_CommHasReceiveData())
    {
    }
}

static size_t getTransmitDataBufferSize()
{
    return g_pTransmitDataBufferCurr - g_pTransmitDataBufferStart;
}

int Platform_CommCausedInterrupt(void)
{
    return 0;
}

void Platform_CommClearInterrupt(void)
{
}

void platformMock_CommSetShareFlag(int flag)
{
    g_commShareFlag = flag;
}

int Platform_CommIsSharedWithApplication(void)
{
    return g_commShareFlag;
}

int Platform_CommIsWaitingForGdbToConnect(void)
{
    return 1;
}

void platformMock_SetInitException(int exceptionToThrow)
{
    g_initException = exceptionToThrow;
}

void platformMock_ClearInitCount()
{
    g_initCount = 0;
    memset(&g_initTokenCopy, 0, sizeof(g_initTokenCopy));
}

int platformMock_GetInitCount()
{
    return g_initCount;
}

Token* platformMock_GetInitTokenCopy()
{
    return &g_initTokenCopy;
}

void Platform_Init(Token* pParameterTokens)
{
    g_initCount++;
    Token_Copy(&g_initTokenCopy, pParameterTokens);
    
    if (g_initException)
        __throw(g_initException);
}

void platformMock_ClearDisableSingleStepCount()
{
    g_disableSingleStepCount = 0;
}

int platformMock_GetDisableSingleStepCount()
{
    return g_disableSingleStepCount;
}

void Platform_DisableSingleStep(void)
{
    g_disableSingleStepCount++;
}















char* Platform_GetPacketBuffer(void)
{
    return NULL;
}

uint32_t  Platform_GetPacketBufferSize(void)
{
    return 0;
}

void Platform_EnteringDebugger(void)
{
}

void Platform_LeavingDebugger(void)
{
}

uint8_t Platform_DetermineCauseOfException(void)
{
    return 0;
}

void Platform_DisplayFaultCauseToGdbConsole(void)
{
}

void Platform_EnableSingleStep(void)
{
}

int Platform_IsSingleStepping(void)
{
    return 0;
}

uint32_t  Platform_GetProgramCounter(void)
{
    return 0;
}

void Platform_SetProgramCounter(uint32_t newPC)
{
    (void)newPC;
}

void Platform_AdvanceProgramCounterToNextInstruction(void)
{
}

int Platform_IsCurrentInstructionHardcodedBreakpoint(void)
{
    return 0;
}

int Platform_WasProgramCounterModifiedByUser(void)
{
    return 0;
}

int Platform_WasMemoryFaultEncountered(void)
{
    return 0;
}

void Platform_WriteTResponseRegistersToBuffer(Buffer* pBuffer)
{
    (void)pBuffer;
}

void Platform_CopyContextToBuffer(Buffer* pBuffer)
{
    (void)pBuffer;
}

void Platform_CopyContextFromBuffer(Buffer* pBuffer)
{
    (void)pBuffer;
}

uint32_t Platform_GetDeviceMemoryMapXmlSize(void)
{
    return 0;
}

const char*  Platform_GetDeviceMemoryMapXml(void)
{
    return NULL;
}

__throws void  Platform_SetHardwareBreakpoint(uint32_t address, uint32_t kind)
{
    (void)address;
    (void)kind;
}

__throws void  Platform_ClearHardwareBreakpoint(uint32_t address, uint32_t kind)
{
    (void)address;
    (void)kind;
}

__throws void  Platform_SetHardwareWatchpoint(uint32_t address, uint32_t size,  PlatformWatchpointType type)
{
    (void)address;
    (void)size;
    (void)type;
}

__throws void  Platform_ClearHardwareWatchpoint(uint32_t address, uint32_t size,  PlatformWatchpointType type)
{
    (void)address;
    (void)size;
    (void)type;
}

PlatformInstructionType Platform_TypeOfCurrentInstruction(void)
{
    return MRI_PLATFORM_INSTRUCTION_OTHER;
}

PlatformSemihostParameters  Platform_GetSemihostCallParameters(void)
{
    PlatformSemihostParameters parameters;;
    
    memset(&parameters, 0, sizeof(parameters));
    
    return parameters;
}

void Platform_SetSemihostCallReturnValue(uint32_t returnValue)
{
    (void)returnValue;
}




int __mriSemihost_IsDebuggeeMakingSemihostCall(void)
{
    return 0;
}

int __mriSemihost_HandleSemihostRequest(void)
{
    return 0;
}
