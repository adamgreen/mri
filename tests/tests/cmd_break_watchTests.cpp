/* Copyright 2014 Adam Green (https://github.com/adamgreen/)

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


TEST_GROUP(cmdBreakWatch)
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

TEST(cmdBreakWatch, SetHardwareBreakpoint)
{
    platformMock_CommInitReceiveChecksummedData("+$Z1,12345678,2#", "+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$OK#+"), platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 1, platformMock_SetHardwareBreakpointCalls() );
    CHECK_EQUAL( 0x12345678, platformMock_SetHardwareBreakpointAddressArg() );
    CHECK_EQUAL( 2, platformMock_SetHardwareBreakpointKindArg() );
}

TEST(cmdBreakWatch, SetHardwareBreakpoint_WithInvalidArgSeparator_ShouldReturnErrorResponse)
{
    platformMock_CommInitReceiveChecksummedData("+$Z1:12345678,2#", "+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$" MRI_ERROR_INVALID_ARGUMENT "#+"),
                   platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 0, platformMock_SetHardwareBreakpointCalls() );
}

TEST(cmdBreakWatch, SetHardwareBreakpoint_ThrowInvalidArgumentException_ShouldReturnErrorResponse)
{
    platformMock_CommInitReceiveChecksummedData("+$Z1,12345678,2#", "+$c#");
    platformMock_SetHardwareBreakpointException(invalidArgumentException);
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$" MRI_ERROR_INVALID_ARGUMENT "#+"),
                   platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 1, platformMock_SetHardwareBreakpointCalls() );
}

TEST(cmdBreakWatch, SetHardwareBreakpoint_ThrowExceededHardwareResourcesException_ShouldReturnErrorResponse)
{
    platformMock_CommInitReceiveChecksummedData("+$Z1,12345678,2#", "+$c#");
    platformMock_SetHardwareBreakpointException(exceededHardwareResourcesException);
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$" MRI_ERROR_NO_FREE_BREAKPOINT "#+"),
                   platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 1, platformMock_SetHardwareBreakpointCalls() );
}

TEST(cmdBreakWatch, SetHardwareBreakpoint_ThrowTimeoutException_ShouldReturnErrorResponse)
{
    platformMock_CommInitReceiveChecksummedData("+$Z1,12345678,2#", "+$c#");
    platformMock_SetHardwareBreakpointException(timeoutException);
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$" MRI_ERROR_NO_FREE_BREAKPOINT "#+"),
                   platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 1, platformMock_SetHardwareBreakpointCalls() );
}

TEST(cmdBreakWatch, SetHardwareWriteWatchpoint)
{
    platformMock_CommInitReceiveChecksummedData("+$Z2,87654321,4#", "+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$OK#+"), platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 1, platformMock_SetHardwareWatchpointCalls() );
    CHECK_EQUAL( 0x87654321, platformMock_SetHardwareWatchpointAddressArg() );
    CHECK_EQUAL( 4, platformMock_SetHardwareWatchpointSizeArg() );
    CHECK_EQUAL( MRI_PLATFORM_WRITE_WATCHPOINT, platformMock_SetHardwareWatchpointTypeArg() );
}

TEST(cmdBreakWatch, SetHardwareWriteWatchpoing_ThrowExceededHardwareResourcesException_ShouldReturnErrorResponse)
{
    platformMock_CommInitReceiveChecksummedData("+$Z2,87654321,4#", "+$c#");
    platformMock_SetHardwareWatchpointException(exceededHardwareResourcesException);
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$" MRI_ERROR_NO_FREE_BREAKPOINT "#+"),
                   platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 1, platformMock_SetHardwareWatchpointCalls() );
    CHECK_EQUAL( MRI_PLATFORM_WRITE_WATCHPOINT, platformMock_SetHardwareWatchpointTypeArg() );
}

TEST(cmdBreakWatch, SetHardwareReadWatchpoint)
{
    platformMock_CommInitReceiveChecksummedData("+$Z3,87654321,8#", "+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$OK#+"), platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 1, platformMock_SetHardwareWatchpointCalls() );
    CHECK_EQUAL( 0x87654321, platformMock_SetHardwareWatchpointAddressArg() );
    CHECK_EQUAL( 8, platformMock_SetHardwareWatchpointSizeArg() );
    CHECK_EQUAL( MRI_PLATFORM_READ_WATCHPOINT, platformMock_SetHardwareWatchpointTypeArg() );
}

TEST(cmdBreakWatch, SetHardwareReadWriteWatchpoint)
{
    platformMock_CommInitReceiveChecksummedData("+$Z4,87654321,8#", "+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$OK#+"), platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 1, platformMock_SetHardwareWatchpointCalls() );
    CHECK_EQUAL( 0x87654321, platformMock_SetHardwareWatchpointAddressArg() );
    CHECK_EQUAL( 8, platformMock_SetHardwareWatchpointSizeArg() );
    CHECK_EQUAL( MRI_PLATFORM_READWRITE_WATCHPOINT, platformMock_SetHardwareWatchpointTypeArg() );
}

TEST(cmdBreakWatch, AttemptSetSoftwareBreakpoint_ShouldReturnEmptyResponse)
{
    platformMock_CommInitReceiveChecksummedData("+$Z0,87654321,8#", "+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$#+"), platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 0, platformMock_SetHardwareBreakpointCalls() );
    CHECK_EQUAL( 0, platformMock_SetHardwareWatchpointCalls() );
}

TEST(cmdBreakWatch, InvalidSetHardwareBreakWatchpoint_ShouldReturnEmptyResponse)
{
    platformMock_CommInitReceiveChecksummedData("+$Z5,87654321,8#", "+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$#+"), platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 0, platformMock_SetHardwareBreakpointCalls() );
    CHECK_EQUAL( 0, platformMock_SetHardwareWatchpointCalls() );
}

TEST(cmdBreakWatch, ClearHardwareBreakpoint)
{
    platformMock_CommInitReceiveChecksummedData("+$z1,12345678,2#", "+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$OK#+"), platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 1, platformMock_ClearHardwareBreakpointCalls() );
    CHECK_EQUAL( 0x12345678, platformMock_ClearHardwareBreakpointAddressArg() );
    CHECK_EQUAL( 2, platformMock_ClearHardwareBreakpointKindArg() );
}

TEST(cmdBreakWatch, ClearHardwareBreakpoint_WithInvalidArgSeparator_ShouldReturnErrorResponse)
{
    platformMock_CommInitReceiveChecksummedData("+$z1:12345678,2#", "+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$" MRI_ERROR_INVALID_ARGUMENT "#+"),
                   platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 0, platformMock_ClearHardwareBreakpointCalls() );
}

TEST(cmdBreakWatch, ClearHardwareBreakpoint_ThrowExceededHardwareResourcesException_ShouldReturnErrorResponse)
{
    platformMock_CommInitReceiveChecksummedData("+$z1,12345678,2#", "+$c#");
    platformMock_ClearHardwareBreakpointException(exceededHardwareResourcesException);
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$" MRI_ERROR_NO_FREE_BREAKPOINT "#+"),
                   platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 1, platformMock_ClearHardwareBreakpointCalls() );
}

TEST(cmdBreakWatch, ClearHardwareWriteWatchpoint)
{
    platformMock_CommInitReceiveChecksummedData("+$z2,87654321,4#", "+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$OK#+"), platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 1, platformMock_ClearHardwareWatchpointCalls() );
    CHECK_EQUAL( 0x87654321, platformMock_ClearHardwareWatchpointAddressArg() );
    CHECK_EQUAL( 4, platformMock_ClearHardwareWatchpointSizeArg() );
    CHECK_EQUAL( MRI_PLATFORM_WRITE_WATCHPOINT, platformMock_ClearHardwareWatchpointTypeArg() );
}

TEST(cmdBreakWatch, ClearHardwareWriteWatchpoing_ThrowExceededHardwareResourcesException_ShouldReturnErrorResponse)
{
    platformMock_CommInitReceiveChecksummedData("+$z2,87654321,4#", "+$c#");
    platformMock_ClearHardwareWatchpointException(exceededHardwareResourcesException);
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$" MRI_ERROR_NO_FREE_BREAKPOINT "#+"),
                   platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 1, platformMock_ClearHardwareWatchpointCalls() );
    CHECK_EQUAL( MRI_PLATFORM_WRITE_WATCHPOINT, platformMock_ClearHardwareWatchpointTypeArg() );
}

TEST(cmdBreakWatch, ClearHardwareReadWatchpoint)
{
    platformMock_CommInitReceiveChecksummedData("+$z3,87654321,8#", "+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$OK#+"), platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 1, platformMock_ClearHardwareWatchpointCalls() );
    CHECK_EQUAL( 0x87654321, platformMock_ClearHardwareWatchpointAddressArg() );
    CHECK_EQUAL( 8, platformMock_ClearHardwareWatchpointSizeArg() );
    CHECK_EQUAL( MRI_PLATFORM_READ_WATCHPOINT, platformMock_ClearHardwareWatchpointTypeArg() );
}

TEST(cmdBreakWatch, ClearHardwareReadWriteWatchpoint)
{
    platformMock_CommInitReceiveChecksummedData("+$z4,87654321,8#", "+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$OK#+"), platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 1, platformMock_ClearHardwareWatchpointCalls() );
    CHECK_EQUAL( 0x87654321, platformMock_ClearHardwareWatchpointAddressArg() );
    CHECK_EQUAL( 8, platformMock_ClearHardwareWatchpointSizeArg() );
    CHECK_EQUAL( MRI_PLATFORM_READWRITE_WATCHPOINT, platformMock_ClearHardwareWatchpointTypeArg() );
}

TEST(cmdBreakWatch, AttemptClearSoftwareBreakpoint_ShouldReturnEmptyResponse)
{
    platformMock_CommInitReceiveChecksummedData("+$z0,87654321,8#", "+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$#+"), platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 0, platformMock_ClearHardwareBreakpointCalls() );
    CHECK_EQUAL( 0, platformMock_ClearHardwareWatchpointCalls() );
}
TEST(cmdBreakWatch, InvalidClearHardwareBreakWatchpoint_ShouldReturnEmptyResponse)
{
    platformMock_CommInitReceiveChecksummedData("+$z5,87654321,8#", "+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$#+"), platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 0, platformMock_ClearHardwareBreakpointCalls() );
    CHECK_EQUAL( 0, platformMock_ClearHardwareWatchpointCalls() );
}