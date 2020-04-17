/* Copyright 2020 Adam Green (https://github.com/adamgreen/)

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

extern "C"
{
#include <core/try_catch.h>
#include <core/mri.h>

void mriDebugException(void);
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
        mriInit("MRI_UART_MBED_USB");
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
        mriDebugException();
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$11111111222222223333333344444444#+"),
                   platformMock_CommGetTransmittedData() );
}

TEST(cmdRegisters, SetRegisters)
{
    platformMock_CommInitReceiveChecksummedData("+$G1234567822222222333333339abcdef0#", "+$c#");
        mriDebugException();
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$OK#+"), platformMock_CommGetTransmittedData() );
    uint32_t* pContext = platformMock_GetContext();
    CHECK_EQUAL ( 0x78563412, pContext[0] );
    CHECK_EQUAL ( 0x22222222, pContext[1] );
    CHECK_EQUAL ( 0x33333333, pContext[2] );
    CHECK_EQUAL ( 0xf0debc9a, pContext[3] );
}

TEST(cmdRegisters, SetRegisters_BufferTooShort)
{
    platformMock_CommInitReceiveChecksummedData("+$G1234567822222222333333339abcdef#", "+$c#");
        mriDebugException();
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$" MRI_ERROR_BUFFER_OVERRUN "#+"),
                   platformMock_CommGetTransmittedData() );
    uint32_t* pContext = platformMock_GetContext();
    CHECK_EQUAL ( 0x78563412, pContext[0] );
    CHECK_EQUAL ( 0x22222222, pContext[1] );
    CHECK_EQUAL ( 0x33333333, pContext[2] );
    CHECK_EQUAL ( 0xffdebc9a, pContext[3] );
}


TEST(cmdRegisters, TResponse_ThreadIdOfZero_ShouldReturnNoThreadId)
{
    platformMock_CommInitReceiveChecksummedData("+$c#");
        mriDebugException();
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+"), platformMock_CommGetTransmittedData() );
}

TEST(cmdRegisters, TResponse_NonZeroThreadId_ShouldReturnThreadId)
{
    platformMock_RtosSetThreadId(0xBAADF00D);
    platformMock_CommInitReceiveChecksummedData("+$c#");
        mriDebugException();
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05thread:baadf00d;responseT#+"), platformMock_CommGetTransmittedData() );
}

TEST(cmdRegisters, TResponse_SoftwareBreakpointHit_ShouldReturnSWBREAKWithNoAddress)
{
    platformMock_CommInitReceiveChecksummedData("+$c#");
    PlatformTrapReason reason = { MRI_PLATFORM_TRAP_TYPE_SWBREAK, 0x1000 };
    platformMock_SetTrapReason(&reason);
        mriDebugException();
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05swbreak:;responseT#+"), platformMock_CommGetTransmittedData() );
}

TEST(cmdRegisters, TResponse_HardwareBreakpointHit_ShouldReturnHWBREAKWithNoAddress)
{
    platformMock_CommInitReceiveChecksummedData("+$c#");
    PlatformTrapReason reason = { MRI_PLATFORM_TRAP_TYPE_HWBREAK, 0x1000 };
    platformMock_SetTrapReason(&reason);
        mriDebugException();
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05hwbreak:;responseT#+"), platformMock_CommGetTransmittedData() );
}

TEST(cmdRegisters, TResponse_WriteWatchpointHit_ShouldReturnWATCHWithAddress)
{
    platformMock_CommInitReceiveChecksummedData("+$c#");
    PlatformTrapReason reason = { MRI_PLATFORM_TRAP_TYPE_WATCH, 0x1000 };
    platformMock_SetTrapReason(&reason);
        mriDebugException();
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05watch:1000;responseT#+"), platformMock_CommGetTransmittedData() );
}

TEST(cmdRegisters, TResponse_ReadWatchpointHit_ShouldReturnRWATCHWithAddress)
{
    platformMock_CommInitReceiveChecksummedData("+$c#");
    PlatformTrapReason reason = { MRI_PLATFORM_TRAP_TYPE_RWATCH, 0x04 };
    platformMock_SetTrapReason(&reason);
        mriDebugException();
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05rwatch:04;responseT#+"), platformMock_CommGetTransmittedData() );
}

TEST(cmdRegisters, TResponse_AccessWatchpointHit_ShouldReturnAWATCHWithAddress)
{
    platformMock_CommInitReceiveChecksummedData("+$c#");
    PlatformTrapReason reason = { MRI_PLATFORM_TRAP_TYPE_AWATCH, 0x80000000 };
    platformMock_SetTrapReason(&reason);
        mriDebugException();
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05awatch:80000000;responseT#+"), platformMock_CommGetTransmittedData() );
}
