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

extern "C"
{
#include <try_catch.h>
#include <mri.h>

void __mriDebugException(void);
}
#include <platformMock.h>

// Include C++ headers for test harness.
#include "CppUTest/TestHarness.h"


TEST_GROUP(cmdRegisters)
{
    int     m_expectedException;            
    
    void setup()
    {
        m_expectedException = noException;
        platformMock_Init();
        __mriInit("MRI_UART_MBED_USB");
    }

    void teardown()
    {
        LONGS_EQUAL ( m_expectedException, getExceptionCode() );
        clearExceptionCode();
        platformMock_Uninit();
    }
    
    void validateExceptionCode(int expectedExceptionCode)
    {
        m_expectedException = expectedExceptionCode;
        LONGS_EQUAL ( expectedExceptionCode, getExceptionCode() );
    }
};

TEST(cmdRegisters, GetRegisters)
{
    uint32_t* pContext = platformMock_GetContext();
    pContext[0] = 0x11111111;
    pContext[1] = 0x22222222;
    pContext[2] = 0x33333333;
    pContext[3] = 0x44444444;
    
    platformMock_CommInitReceiveChecksummedData("+$g#", "+$c#");
        __mriDebugException();
    CHECK_TRUE ( platformMock_CommDoesTransmittedDataEqual("$T05responseT#7c+$11111111222222223333333344444444#50+") );
}

TEST(cmdRegisters, SetRegisters)
{
    platformMock_CommInitReceiveChecksummedData("+$G1234567822222222333333339abcdef0#", "+$c#");
        __mriDebugException();
    CHECK_TRUE ( platformMock_CommDoesTransmittedDataEqual("$T05responseT#7c+$OK#9a+") );
    uint32_t* pContext = platformMock_GetContext();
    CHECK_EQUAL ( 0x78563412, pContext[0] );    
    CHECK_EQUAL ( 0x22222222, pContext[1] );    
    CHECK_EQUAL ( 0x33333333, pContext[2] );    
    CHECK_EQUAL ( 0xf0debc9a, pContext[3] );    
}

TEST(cmdRegisters, SetRegisters_BufferTooShort)
{
    platformMock_CommInitReceiveChecksummedData("+$G1234567822222222333333339abcdef#", "+$c#");
        __mriDebugException();
    CHECK_TRUE ( platformMock_CommDoesTransmittedDataEqual("$T05responseT#7c+$" MRI_ERROR_BUFFER_OVERRUN "#a9+") );
    uint32_t* pContext = platformMock_GetContext();
    CHECK_EQUAL ( 0x78563412, pContext[0] );    
    CHECK_EQUAL ( 0x22222222, pContext[1] );    
    CHECK_EQUAL ( 0x33333333, pContext[2] );    
    CHECK_EQUAL ( 0xffdebc9a, pContext[3] );    
}
