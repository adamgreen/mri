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
#include <core/gdb_console.h>
}
#include <platformMock.h>

// Include C++ headers for test harness.
#include "CppUTest/TestHarness.h"


TEST_GROUP(gdbConsole)
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

TEST(gdbConsole, WriteStringToGdbConsole_SendAsGdbPacket)
{
    WriteStringToGdbConsole("Test\n");
    CHECK_TRUE ( platformMock_CommDoesTransmittedDataEqual("$O546573740a#89") );
}

TEST(gdbConsole, WriteStringToGdbConsole_FailToSendAsGdbPacketBecauseOfSmallBuffer)
{
    platformMock_SetPacketBufferSize(10);
    WriteStringToGdbConsole("Test\n");
    CHECK_TRUE ( platformMock_CommDoesTransmittedDataEqual("") );
    validateExceptionCode(bufferOverrunException);
}

TEST(gdbConsole, WriteHexValueToGdbConsole_SendMinimumValue)
{
    WriteHexValueToGdbConsole(0);
    CHECK_TRUE ( platformMock_CommDoesTransmittedDataEqual("$O30783030#e7") );
}

TEST(gdbConsole, WriteHexValueToGdbConsole_SendMaximumValue)
{
    WriteHexValueToGdbConsole(~0U);
    CHECK_TRUE ( platformMock_CommDoesTransmittedDataEqual("$O30786666666666666666#81") );
}
