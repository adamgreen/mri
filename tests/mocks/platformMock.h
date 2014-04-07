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

void        platformMock_Init(void);
void        platformMock_Uninit(void);

void        platformMock_CommInitReceiveData(const char* pDataToReceive);
void        platformMock_CommInitTransmitDataBuffer(size_t Size);
void        platformMock_CommUninitTransmitDataBuffer(void);
int         platformMock_CommDoesTransmittedDataEqual(const char* thisString);
void        platformMock_CommSetShareFlag(int flag);

void        platformMock_SetInitException(int exceptionToThrow);
void        platformMock_ClearInitCount(void);
int         platformMock_GetInitCount(void);
Token*      platformMock_GetInitTokenCopy(void);

void        platformMock_ClearDisableSingleStepCount(void);
int         platformMock_GetDisableSingleStepCount(void);

#endif /* _PLATFORM_MOCK_H_ */
