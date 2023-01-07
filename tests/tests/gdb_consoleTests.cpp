/* Copyright 2022 Adam Green (https://github.com/adamgreen/)

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
    size_t charsWritten = WriteStringToGdbConsole("Test\n");
    LONGS_EQUAL( 5, charsWritten );
    STRCMP_EQUAL ( platformMock_CommChecksumData("$O546573740a#"), platformMock_CommGetTransmittedData() );
}

TEST(gdbConsole, WriteStringToGdbConsole_TruncateWriteBecauseOfSmallBuffer)
{
    platformMock_SetPacketBufferSize(1+2*4+4);
    size_t charsWritten = WriteStringToGdbConsole("Test\n");
    LONGS_EQUAL ( 4, charsWritten );
    STRCMP_EQUAL ( platformMock_CommChecksumData("$O54657374#"), platformMock_CommGetTransmittedData() );
}

TEST(gdbConsole, WriteHexValueToGdbConsole_SendMinimumValue)
{
    WriteHexValueToGdbConsole(0);
    STRCMP_EQUAL ( platformMock_CommChecksumData("$O30783030#"), platformMock_CommGetTransmittedData() );
}

TEST(gdbConsole, WriteHexValueToGdbConsole_SendMaximumValue)
{
    WriteHexValueToGdbConsole(~0U);
    STRCMP_EQUAL ( platformMock_CommChecksumData("$O30786666666666666666#"), platformMock_CommGetTransmittedData() );
}
