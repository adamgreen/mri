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
#include <gdb_console.h>
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

TEST(gdbConsole, WriteStringToGdbConsole_SendAsGdbPacketWhenNotShared)
{
    platformMock_SetCommSharingWithApplication(0);
    WriteStringToGdbConsole("Test\n");
    CHECK_TRUE ( platformMock_CommDoesTransmittedDataEqual("$O546573740a#89") );
}

TEST(gdbConsole, WriteStringToGdbConsole_SendRawStringWhenShared)
{
    platformMock_SetCommSharingWithApplication(1);
    WriteStringToGdbConsole("Test string\n");
    CHECK_TRUE ( platformMock_CommDoesTransmittedDataEqual("Test string\n") );
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
    platformMock_SetCommSharingWithApplication(1);
    WriteHexValueToGdbConsole(0);
    CHECK_TRUE ( platformMock_CommDoesTransmittedDataEqual("0x00") );
}

TEST(gdbConsole, WriteHexValueToGdbConsole_SendMaximumValue)
{
    platformMock_SetCommSharingWithApplication(1);
    WriteHexValueToGdbConsole(~0U);
    CHECK_TRUE ( platformMock_CommDoesTransmittedDataEqual("0xffffffff") );
}