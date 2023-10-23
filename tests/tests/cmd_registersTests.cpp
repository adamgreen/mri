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

extern "C"
{
#include <core/try_catch.h>
#include <core/mri.h>
#include <core/core.h>
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
    uintmri_t* pContext = platformMock_GetContextEntries();
    pContext[0] = 0x1111111111111111;
    pContext[1] = 0x2222222222222222;
    pContext[2] = 0x3333333333333333;
    pContext[3] = 0x4444444444444444;

    platformMock_CommInitReceiveChecksummedData("+$g#", "+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$1111111111111111222222222222222233333333333333334444444444444444#+"),
                   platformMock_CommGetTransmittedData() );
}

TEST(cmdRegisters, SetRegisters)
{
    platformMock_CommInitReceiveChecksummedData("+$G123456789abcdef0222222222222222233333333333333339abcdef012345678#", "+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$OK#+"), platformMock_CommGetTransmittedData() );
    uintmri_t* pContext = platformMock_GetContextEntries();
    LONGS_EQUAL ( 0xF0DEBC9A78563412, pContext[0] );
    LONGS_EQUAL ( 0x2222222222222222, pContext[1] );
    LONGS_EQUAL ( 0x3333333333333333, pContext[2] );
    LONGS_EQUAL ( 0x78563412f0debc9a, pContext[3] );
}

TEST(cmdRegisters, SetRegisters_BufferTooShort)
{
    platformMock_CommInitReceiveChecksummedData("+$G123456789abcdef022222222222222223333333333333333fedcba987654321#", "+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$" MRI_ERROR_BUFFER_OVERRUN "#+"),
                   platformMock_CommGetTransmittedData() );
    uintmri_t* pContext = platformMock_GetContextEntries();
    LONGS_EQUAL ( 0xF0DEBC9A78563412, pContext[0] );
    LONGS_EQUAL ( 0x2222222222222222, pContext[1] );
    LONGS_EQUAL ( 0x3333333333333333, pContext[2] );
    LONGS_EQUAL ( 0x0032547698BADCFE, pContext[3] );
}


TEST(cmdRegisters, TResponse_ThreadIdOfZero_ShouldReturnNoThreadId)
{
    platformMock_CommInitReceiveChecksummedData("+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+"), platformMock_CommGetTransmittedData() );
}

TEST(cmdRegisters, TResponse_NonZeroThreadId_ShouldReturnThreadId)
{
    platformMock_RtosSetHaltedThreadId(0xBAADF00D);
    platformMock_CommInitReceiveChecksummedData("+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05thread:baadf00d;responseT#+"), platformMock_CommGetTransmittedData() );
}

TEST(cmdRegisters, TResponse_WriteWatchpointHit_ShouldReturnWATCHWithAddress)
{
    platformMock_CommInitReceiveChecksummedData("+$c#");
    PlatformTrapReason reason = { MRI_PLATFORM_TRAP_TYPE_WATCH, 0x1000 };
    platformMock_SetTrapReason(&reason);
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05watch:1000;responseT#+"), platformMock_CommGetTransmittedData() );
}

TEST(cmdRegisters, TResponse_ReadWatchpointHit_ShouldReturnRWATCHWithAddress)
{
    platformMock_CommInitReceiveChecksummedData("+$c#");
    PlatformTrapReason reason = { MRI_PLATFORM_TRAP_TYPE_RWATCH, 0x04 };
    platformMock_SetTrapReason(&reason);
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05rwatch:04;responseT#+"), platformMock_CommGetTransmittedData() );
}

TEST(cmdRegisters, TResponse_AccessWatchpointHit_ShouldReturnAWATCHWithAddress)
{
    platformMock_CommInitReceiveChecksummedData("+$c#");
    PlatformTrapReason reason = { MRI_PLATFORM_TRAP_TYPE_AWATCH, 0x80000000 };
    platformMock_SetTrapReason(&reason);
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05awatch:80000000;responseT#+"), platformMock_CommGetTransmittedData() );
}
