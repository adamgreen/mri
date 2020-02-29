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
#include <core/core.h>
#include <core/token.h>
#include <core/platforms.h>

void __mriDebugException(void);
}
#include <platformMock.h>

// Include C++ headers for test harness.
#include "CppUTest/TestHarness.h"

static bool     g_callbackCalled;
static int      g_callbackReturnValue;
static void*    g_pvCallbackContext;


TEST_GROUP(Mri)
{
    int     m_expectException;

    void setup()
    {
        m_expectException = noException;
        g_callbackCalled = false;
        g_pvCallbackContext = (void*)-1;
        platformMock_Init();
    }

    void teardown()
    {
        LONGS_EQUAL ( m_expectException, getExceptionCode() );
        clearExceptionCode();
        platformMock_Uninit();
    }

    void validateExceptionCode(int expectedExceptionCode)
    {
        m_expectException = expectedExceptionCode;
        LONGS_EQUAL ( expectedExceptionCode, getExceptionCode() );
    }
};

TEST(Mri, __mriInit_MakeSureThatItCallsMriPlatformInit)
{
    __mriInit("MRI_UART_MBED_USB");
    LONGS_EQUAL( 1, platformMock_GetInitCount() );
}

TEST(Mri, __mriInit_MakeSureThatItPassesTokenizedStringIntoMriPlatformInit)
{
    __mriInit("MRI_UART_MBED_USB MRI_UART_SHARE");

    Token* pInitTokens = platformMock_GetInitTokenCopy();
    LONGS_EQUAL( 2, Token_GetTokenCount(pInitTokens) );
    STRCMP_EQUAL( "MRI_UART_MBED_USB", Token_GetToken(pInitTokens, 0) );
    STRCMP_EQUAL( "MRI_UART_SHARE", Token_GetToken(pInitTokens, 1) );
}

TEST(Mri, __mriInit_MakeSureThatItSetsProperFlagsOnSuccessfulInit)
{
    __mriInit("MRI_UART_MBED_USB");
    CHECK_TRUE( IsFirstException() );
    CHECK_TRUE( WasSuccessfullyInit() );
}

TEST(Mri, __mriInit_HandlesMriPlatformInitThrowingException)
{
    platformMock_SetInitException(timeoutException);
    __mriInit("MRI_UART_MBED_USB");
    validateExceptionCode(timeoutException);
    CHECK_FALSE( IsFirstException() );
    CHECK_FALSE( WasSuccessfullyInit() );
}

TEST(Mri, __mriDebugExceptionShouldJustClearCommInterruptAndReturnWithoutEnteringDebuggerIfInterruptActiveAndNoData)
{
    __mriInit("MRI_UART_MBED_USB");
    platformMock_CommInitReceiveData("");
    platformMock_CommSetInterruptBit(1);
        __mriDebugException();
    CHECK_FALSE( Platform_CommCausedInterrupt() );
    CHECK_EQUAL( 0, platformMock_GetHandleSemihostRequestCalls() );
    CHECK_EQUAL( 0, platformMock_GetEnteringDebuggerCalls() );
    CHECK_EQUAL( 0, platformMock_GetLeavingDebuggerCalls() );
    CHECK_TRUE( IsFirstException() );
}

TEST(Mri, __mriDebugExceptionShouldEnterAndLeaveIfHandlingSemihostRequest_NoWaitForGdbToConnect)
{
    __mriInit("MRI_UART_MBED_USB");
    platformMock_SetIsDebuggeeMakingSemihostCall(1);
        __mriDebugException();
    CHECK_EQUAL( 1, platformMock_GetHandleSemihostRequestCalls() );
    CHECK_EQUAL( 1, platformMock_GetEnteringDebuggerCalls() );
    CHECK_EQUAL( 1, platformMock_GetLeavingDebuggerCalls() );
    CHECK_FALSE( IsFirstException() );
    CHECK_EQUAL( 0, platformMock_GetCommWaitForReceiveDataToStopCalls() );
    CHECK_EQUAL( 0, platformMock_GetCommPrepareToWaitForGdbConnectionCalls() );
    CHECK_EQUAL( 0, platformMock_DisplayFaultCauseToGdbConsoleCalls() );
}

TEST(Mri, __mriDebugExceptionShouldEnterAndLeaveIfHandlingSemihostRequest_WaitAndHaveGdbConnectOnFirstByte)
{
    __mriInit("MRI_UART_MBED_USB");
    platformMock_SetIsDebuggeeMakingSemihostCall(1);
    platformMock_CommSetShouldWaitForGdbConnect(1);
    platformMock_CommInitReceiveData("+");
        __mriDebugException();
    CHECK_EQUAL( 1, platformMock_GetHandleSemihostRequestCalls() );
    CHECK_EQUAL( 1, platformMock_GetEnteringDebuggerCalls() );
    CHECK_EQUAL( 1, platformMock_GetLeavingDebuggerCalls() );
    CHECK_FALSE( IsFirstException() );
    CHECK_EQUAL( 0, platformMock_GetCommWaitForReceiveDataToStopCalls() );
    CHECK_EQUAL( 0, platformMock_GetCommPrepareToWaitForGdbConnectionCalls() );
    CHECK_EQUAL( 0, platformMock_DisplayFaultCauseToGdbConsoleCalls() );
}

TEST(Mri, __mriDebugExceptionShouldEnterAndLeaveIfHandlingSemihostRequest_WaitAndHaveGdbConnectOnFourthByte)
{
    __mriInit("MRI_UART_MBED_USB");
    platformMock_SetIsDebuggeeMakingSemihostCall(1);
    platformMock_CommSetShouldWaitForGdbConnect(1);
    platformMock_CommSetIsWaitingForGdbToConnectIterations(2);
    platformMock_CommInitReceiveData("123+");
        __mriDebugException();
    CHECK_EQUAL( 1, platformMock_GetHandleSemihostRequestCalls() );
    CHECK_EQUAL( 1, platformMock_GetEnteringDebuggerCalls() );
    CHECK_EQUAL( 1, platformMock_GetLeavingDebuggerCalls() );
    CHECK_FALSE( IsFirstException() );
    CHECK_EQUAL( 3, platformMock_GetCommWaitForReceiveDataToStopCalls() );
    CHECK_EQUAL( 3, platformMock_GetCommPrepareToWaitForGdbConnectionCalls() );
    CHECK_EQUAL( 0, platformMock_DisplayFaultCauseToGdbConsoleCalls() );
}

TEST(Mri, __mriDebugExceptionShouldDumpExceptionResultAndTResponseAndParseCommands_NoWaitForGdbToConnect_SentContinueCommand)
{
    __mriInit("MRI_UART_MBED_USB");
    platformMock_CommInitReceiveChecksummedData("+$c#");
        __mriDebugException();
    CHECK_EQUAL( 0, platformMock_GetHandleSemihostRequestCalls() );
    CHECK_EQUAL( 1, platformMock_GetEnteringDebuggerCalls() );
    CHECK_EQUAL( 0, platformMock_GetCommWaitForReceiveDataToStopCalls() );
    CHECK_EQUAL( 0, platformMock_GetCommPrepareToWaitForGdbConnectionCalls() );
    CHECK_EQUAL( 1, platformMock_DisplayFaultCauseToGdbConsoleCalls() );
    CHECK_TRUE ( platformMock_CommDoesTransmittedDataEqual("$T05responseT#7c+") );
    CHECK_EQUAL( 1, platformMock_GetLeavingDebuggerCalls() );
    CHECK_FALSE( IsFirstException() );
}

TEST(Mri, __mriDebugExceptionShouldSkipExceptionDumpAndTResponseSinceWaitingForGdbToConnect_SentContinueCommand)
{
    __mriInit("MRI_UART_MBED_USB");
    platformMock_CommSetShouldWaitForGdbConnect(1);
    platformMock_CommInitReceiveChecksummedData("123+$c#");
        __mriDebugException();
    CHECK_EQUAL( 0, platformMock_GetHandleSemihostRequestCalls() );
    CHECK_EQUAL( 1, platformMock_GetEnteringDebuggerCalls() );
    CHECK_EQUAL( 3, platformMock_GetCommWaitForReceiveDataToStopCalls() );
    CHECK_EQUAL( 3, platformMock_GetCommPrepareToWaitForGdbConnectionCalls() );
    CHECK_EQUAL( 0, platformMock_DisplayFaultCauseToGdbConsoleCalls() );
    CHECK_TRUE ( platformMock_CommDoesTransmittedDataEqual("+") );
    CHECK_EQUAL( 1, platformMock_GetLeavingDebuggerCalls() );
    CHECK_FALSE( IsFirstException() );
}

TEST(Mri, __mriDebugException_SentStatusAndContinueCommands)
{
    __mriInit("MRI_UART_MBED_USB");
    platformMock_CommInitReceiveChecksummedData("+$?#", "+$c#");
        __mriDebugException();
    CHECK_TRUE ( platformMock_CommDoesTransmittedDataEqual("$T05responseT#7c" "+$T05responseT#7c" "+") );
}

TEST(Mri, __mriDebugException_WhenSentInvalidCommand_ReturnsEmptyPacketResponse)
{
    __mriInit("MRI_UART_MBED_USB");
    platformMock_CommInitReceiveChecksummedData("+$*#", "+$c#");
        __mriDebugException();
    CHECK_TRUE ( platformMock_CommDoesTransmittedDataEqual("$T05responseT#7c" "+$#00" "+") );
}


TEST(Mri, __mriDebugException_PacketBufferTooSmallShouldResultInBufferOverrunError)
{
    __mriInit("MRI_UART_MBED_USB");
    platformMock_CommInitReceiveChecksummedData("+$?#", "+$c#");
    platformMock_SetPacketBufferSize(11);
        __mriDebugException();
    CHECK_TRUE ( platformMock_CommDoesTransmittedDataEqual("$" MRI_ERROR_BUFFER_OVERRUN "#a9"
                                                           "+$" MRI_ERROR_BUFFER_OVERRUN "#a9" "+") );
}




TEST(Mri, __mriCoreSetTempBreakpoint_ShouldSucceedAndSetHardwareBreakpointAtSpecifiedAddressWithThumbBitCleared)
{
    __mriInit("MRI_UART_MBED_USB");

    CHECK_TRUE( __mriCore_SetTempBreakpoint(0xBAADF00D, NULL, NULL) );
    CHECK_EQUAL( 1, platformMock_SetHardwareBreakpointCalls() );
    CHECK_EQUAL( 0xBAADF00D & ~1, platformMock_SetHardwareBreakpointAddressArg() );
    CHECK_EQUAL( 0xFFFFFFFF, platformMock_SetHardwareBreakpointKindArg() );
}

TEST(Mri, __mriCoreSetTempBreakpoint_TryToSetBreakpointTwiceAndSecondCallFails)
{
    __mriInit("MRI_UART_MBED_USB");

    CHECK_TRUE( __mriCore_SetTempBreakpoint(0xBAADF00D, NULL, NULL) );
    CHECK_EQUAL( 1, platformMock_SetHardwareBreakpointCalls() );
    CHECK_EQUAL( 0xBAADF00D & ~1, platformMock_SetHardwareBreakpointAddressArg() );
    CHECK_EQUAL( 0xFFFFFFFF, platformMock_SetHardwareBreakpointKindArg() );

    CHECK_FALSE( __mriCore_SetTempBreakpoint(0xBAADF00D, NULL, NULL) );
    // Should still just have hardware breakpoint from first successful call.
    CHECK_EQUAL( 1, platformMock_SetHardwareBreakpointCalls() );
    CHECK_EQUAL( 0xBAADF00D & ~1, platformMock_SetHardwareBreakpointAddressArg() );
    CHECK_EQUAL( 0xFFFFFFFF, platformMock_SetHardwareBreakpointKindArg() );
}

TEST(Mri, __mriCoreSetTempBreakpoint_SetHardwareBreakpointThrows_ShouldFail_ExceptionShouldBeCleared)
{
    __mriInit("MRI_UART_MBED_USB");

    platformMock_SetHardwareBreakpointException(exceededHardwareResourcesException);
    CHECK_FALSE( __mriCore_SetTempBreakpoint(0xBAADF00D, NULL, NULL) );
    CHECK_EQUAL( noException, getExceptionCode() );
}

TEST(Mri, __mriCoreSetTempBreakpoint_RunMriDebugException_DontHitBreakpoint_BreakpointShouldNotBeCleared)
{
    __mriInit("MRI_UART_MBED_USB");

    __mriCore_SetTempBreakpoint(0xBAADF00D, NULL, NULL);

    __mriPlatform_SetProgramCounter(0xBAADF00B);
    platformMock_CommInitReceiveChecksummedData("+$c#");
        __mriDebugException();
    CHECK_TRUE ( platformMock_CommDoesTransmittedDataEqual("$T05responseT#7c+") );

    CHECK_EQUAL( 0xBAADF00D & ~1, platformMock_SetHardwareBreakpointAddressArg() );
    CHECK_EQUAL( 0, platformMock_ClearHardwareBreakpointCalls() );
}

TEST(Mri, __mriCoreSetTempBreakpoint_RunMriDebugException_HitBreakpoint_BreakpointShouldBeCleared)
{
    __mriInit("MRI_UART_MBED_USB");

    __mriCore_SetTempBreakpoint(0xBAADF00D, NULL, NULL);

    __mriPlatform_SetProgramCounter(0xBAADF00D);
    platformMock_CommInitReceiveChecksummedData("+$c#");
        __mriDebugException();
    CHECK_TRUE ( platformMock_CommDoesTransmittedDataEqual("$T05responseT#7c+") );

    CHECK_EQUAL( 1, platformMock_ClearHardwareBreakpointCalls() );
}

TEST(Mri, __mriCoreSetTempBreakpoint_RunMriDebugException_HitBreakpoint_ClearBreakpointThrows_BreakpointShouldStillBeCleared)
{
    __mriInit("MRI_UART_MBED_USB");

    __mriCore_SetTempBreakpoint(0xBAADF00D, NULL, NULL);

    __mriPlatform_SetProgramCounter(0xBAADF00D);
    platformMock_ClearHardwareBreakpointException(memFaultException);
    platformMock_CommInitReceiveChecksummedData("+$c#");
        __mriDebugException();
    CHECK_TRUE ( platformMock_CommDoesTransmittedDataEqual("$T05responseT#7c+") );

    CHECK_EQUAL( 1, platformMock_ClearHardwareBreakpointCalls() );
}

static int testCallback(void* pvContext){
    g_callbackCalled = true;
    g_pvCallbackContext = pvContext;
    return g_callbackReturnValue;
}

TEST(Mri, __mriCoreSetTempBreakpoint_RunMriDebugException_WithCallbackReturning0_ShouldContinueToCommandParser)
{
    __mriInit("MRI_UART_MBED_USB");

    __mriCore_SetTempBreakpoint(0xBAADF00D, testCallback, (void*)this);

    g_callbackReturnValue = 0;
    __mriPlatform_SetProgramCounter(0xBAADF00D);
    platformMock_CommInitReceiveChecksummedData("+$c#");
        __mriDebugException();
    CHECK_TRUE ( platformMock_CommDoesTransmittedDataEqual("$T05responseT#7c+") );

    CHECK_EQUAL( 1, platformMock_ClearHardwareBreakpointCalls() );
    CHECK_TRUE( g_callbackCalled );
    POINTERS_EQUAL( this, g_pvCallbackContext );
}

TEST(Mri, __mriCoreSetTempBreakpoint_RunMriDebugException_WithCallbackReturning1_ShouldReturnImmediately)
{
    __mriInit("MRI_UART_MBED_USB");

    __mriCore_SetTempBreakpoint(0xBAADF00D, testCallback, NULL);

    g_callbackReturnValue = 1;
    __mriPlatform_SetProgramCounter(0xBAADF00D);
    platformMock_CommInitReceiveChecksummedData("+$c#");
        __mriDebugException();
    CHECK_TRUE ( platformMock_CommDoesTransmittedDataEqual("") );

    CHECK_EQUAL( 1, platformMock_ClearHardwareBreakpointCalls() );
    CHECK_TRUE( g_callbackCalled );
    POINTERS_EQUAL( NULL, g_pvCallbackContext );
}
