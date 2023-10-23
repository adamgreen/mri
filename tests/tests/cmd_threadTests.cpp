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
#include <core/context.h>
}
#include <platformMock.h>

// Include C++ headers for test harness.
#include "CppUTest/TestHarness.h"


TEST_GROUP(cmdThread)
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

TEST(cmdThread, HCommand_NoOperation_ShouldReturnInvalidArgumentError)
{
    platformMock_CommInitReceiveChecksummedData("+$H#", "+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$" MRI_ERROR_INVALID_ARGUMENT "#+"),
                   platformMock_CommGetTransmittedData() );
}

TEST(cmdThread, HCommand_InvalidOperation_ShouldReturnInvalidArgumentError)
{
    platformMock_CommInitReceiveChecksummedData("+$Hx#", "+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$" MRI_ERROR_INVALID_ARGUMENT "#+"),
                   platformMock_CommGetTransmittedData() );
}

TEST(cmdThread, HCommand_InvalidHexDigitInThreadId_ShouldReturnInvalidArgumentError)
{
    platformMock_CommInitReceiveChecksummedData("+$Hgxxxxxxxx#", "+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$" MRI_ERROR_INVALID_ARGUMENT "#+"),
                   platformMock_CommGetTransmittedData() );
}

TEST(cmdThread, HCommand_InvalidThreadId_ShouldReturnInvalidArgumentError)
{
    platformMock_CommInitReceiveChecksummedData("+$Hgbaadbeef#", "+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$" MRI_ERROR_INVALID_ARGUMENT "#+"),
                   platformMock_CommGetTransmittedData() );
}

TEST(cmdThread, HCommand_UseValidContext_VerifyRegisterRead)
{
    uintmri_t contextEntries[4] =
    {
        0x1111111111111111,
        0x2222222222222222,
        0x3333333333333333,
        0x4444444444444444
    };
    ContextSection section = { .pValues = contextEntries, .count = sizeof(contextEntries)/sizeof(contextEntries[0]) };
    MriContext context;
    Context_Init(&context, &section, 1);
    platformMock_RtosSetThreadContext(0xbaadbeef, &context);

    platformMock_CommInitReceiveChecksummedData("+$Hgbaadbeef#", "+$g#", "+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+" "$OK#"
                                                 "+$1111111111111111222222222222222233333333333333334444444444444444#" "+"),
                   platformMock_CommGetTransmittedData() );
}

TEST(cmdThread, HCommand_UseValidContext_VerifyRegisterWrite)
{
    uintmri_t contextEntries[4] =
    {
        0x1111111111111111,
        0x2222222222222222,
        0x3333333333333333,
        0x4444444444444444
    };
    ContextSection section = { .pValues = contextEntries, .count = sizeof(contextEntries)/sizeof(contextEntries[0]) };
    MriContext context;
    Context_Init(&context, &section, 1);
    platformMock_RtosSetThreadContext(0xbaadbeef, &context);

    platformMock_CommInitReceiveChecksummedData("+$Hgbaadbeef#", "+$Gaaaaaaaaaaaaaaaabbbbbbbbbbbbbbbbccccccccccccccccdddddddddddddddd#", "+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#" "+$OK#" "+$OK#" "+"),
                   platformMock_CommGetTransmittedData() );

    LONGS_EQUAL ( 0xAAAAAAAAAAAAAAAA, contextEntries[0] );
    LONGS_EQUAL ( 0xBBBBBBBBBBBBBBBB, contextEntries[1] );
    LONGS_EQUAL ( 0xCCCCCCCCCCCCCCCC, contextEntries[2] );
    LONGS_EQUAL ( 0xDDDDDDDDDDDDDDDD, contextEntries[3] );
}



TEST(cmdThread, TCommand_InactiveThreadSpecified_ShouldReturnError)
{
    platformMock_CommInitReceiveChecksummedData("+$Tbaadbeef#", "+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#" "+$" MRI_ERROR_INVALID_ARGUMENT "#" "+"),
                   platformMock_CommGetTransmittedData() );
}

TEST(cmdThread, TCommand_InvalidHexDigitsInThreadId_ShouldReturnError)
{
    platformMock_CommInitReceiveChecksummedData("+$Tbaadxxxx#", "+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#" "+$" MRI_ERROR_INVALID_ARGUMENT "#" "+"),
                   platformMock_CommGetTransmittedData() );
}

TEST(cmdThread, TCommand_ActiveThreadSpecified_ShouldReturnOK)
{
    platformMock_RtosSetActiveThread(0xBAADBEEF);
    platformMock_CommInitReceiveChecksummedData("+$Tbaadbeef#", "+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#" "+$OK#" "+"),
                   platformMock_CommGetTransmittedData() );
}
