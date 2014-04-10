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
#ifndef _PLATFORM_MOCK_H_
#define _PLATFORM_MOCK_H_

#include <token.h>
#include <platforms.h>

#define INITIAL_PC 0x10000000

void        platformMock_Init(void);
void        platformMock_Uninit(void);

void        platformMock_CommInitReceiveData(const char* pDataToReceive1, const char* pDataToReceive2 = NULL);
void        platformMock_CommInitReceiveChecksummedData(const char* pDataToReceive1, const char* pDataToReceive2 = NULL);
void        platformMock_CommInitTransmitDataBuffer(size_t Size);
int         platformMock_CommDoesTransmittedDataEqual(const char* thisString);
void        platformMock_CommSetInterruptBit(int setValue);
void        platformMock_CommSetShouldWaitForGdbConnect(int setValue);
void        platformMock_CommSetIsWaitingForGdbToConnectIterations(int iterations);
int         platformMock_GetCommWaitForReceiveDataToStopCalls(void);
int         platformMock_GetCommPrepareToWaitForGdbConnectionCalls(void);
void        platformMock_SetCommSharingWithApplication(int setValue);

void        platformMock_SetInitException(int exceptionToThrow);
int         platformMock_GetInitCount(void);
Token*      platformMock_GetInitTokenCopy(void);

int         platformMock_GetEnteringDebuggerCalls(void);
int         platformMock_GetLeavingDebuggerCalls(void);

void        platformMock_SetIsDebuggeeMakingSemihostCall(int setValue);
int         platformMock_GetHandleSemihostRequestCalls(void);

int         platformMock_DisplayFaultCauseToGdbConsoleCalls(void);

void        platformMock_SetPacketBufferSize(uint32_t setValue);

void        platformMock_SetTypeOfCurrentInstruction(PlatformInstructionType setValue);
int         platformMock_AdvanceProgramCounterToNextInstructionCalls(void);
int         platformMock_SetProgramCounterCalls(void);
uint32_t    platformMock_GetProgramCounterValue(void);

#endif /* _PLATFORM_MOCK_H_ */
