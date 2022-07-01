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
#include <core/core.h>
#include <core/token.h>
#include <core/platforms.h>
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

TEST(Mri, mriInit_MakeSureThatItCallsMriPlatformInit)
{
    mriInit("MRI_UART_MBED_USB");
    LONGS_EQUAL( 1, platformMock_GetInitCount() );
}

TEST(Mri, mriInit_MakeSureThatItPassesTokenizedStringIntoMriPlatformInit)
{
    mriInit("MRI_UART_MBED_USB MRI_UART_SHARE");

    Token* pInitTokens = platformMock_GetInitTokenCopy();
    LONGS_EQUAL( 2, Token_GetTokenCount(pInitTokens) );
    STRCMP_EQUAL( "MRI_UART_MBED_USB", Token_GetToken(pInitTokens, 0) );
    STRCMP_EQUAL( "MRI_UART_SHARE", Token_GetToken(pInitTokens, 1) );
}

TEST(Mri, mriInit_MakeSureThatItSetsProperFlagsOnSuccessfulInit)
{
    mriInit("MRI_UART_MBED_USB");
    CHECK_TRUE( IsFirstException() );
    CHECK_TRUE( WasSuccessfullyInit() );
}

TEST(Mri, mriInit_HandlesMriPlatformInitThrowingException)
{
    platformMock_SetInitException(timeoutException);
    mriInit("MRI_UART_MBED_USB");
    validateExceptionCode(timeoutException);
    CHECK_FALSE( IsFirstException() );
    CHECK_FALSE( WasSuccessfullyInit() );
}

TEST(Mri, mriDebugExceptionShouldEnterAndLeaveIfHandlingSemihostRequest_NoWaitForGdbToConnect)
{
    mriInit("MRI_UART_MBED_USB");
    platformMock_SetIsDebuggeeMakingSemihostCall(1);
        mriDebugException(platformMock_GetContext());
    CHECK_EQUAL( 1, platformMock_GetHandleSemihostRequestCalls() );
    CHECK_EQUAL( 1, platformMock_GetEnteringDebuggerCalls() );
    CHECK_EQUAL( 1, platformMock_GetLeavingDebuggerCalls() );
    CHECK_FALSE( IsFirstException() );
    CHECK_EQUAL( 0, platformMock_DisplayFaultCauseToGdbConsoleCalls() );
}

TEST(Mri, mriDebugExceptionOnFirstExceptionShouldSendTResponseAndParseCommands_SentContinueCommand)
{
    mriInit("MRI_UART_MBED_USB");
    platformMock_CommInitReceiveChecksummedData("+$c#");
        mriDebugException(platformMock_GetContext());
    CHECK_EQUAL( 0, platformMock_GetHandleSemihostRequestCalls() );
    CHECK_EQUAL( 1, platformMock_GetEnteringDebuggerCalls() );
    CHECK_EQUAL( 0, platformMock_DisplayFaultCauseToGdbConsoleCalls() );
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+"), platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 1, platformMock_GetLeavingDebuggerCalls() );
    CHECK_FALSE( IsFirstException() );
}

TEST(Mri, mriDebugExceptionOnSecondExceptionShouldDumpExceptionAndSendTResponseAndParseCommands_SentContinueCommand)
{
    mriInit("MRI_UART_MBED_USB");
    platformMock_CommInitReceiveChecksummedData("+$c#");
        mriDebugException(platformMock_GetContext());
    CHECK_EQUAL( 0, platformMock_GetHandleSemihostRequestCalls() );
    CHECK_EQUAL( 1, platformMock_GetEnteringDebuggerCalls() );
    CHECK_EQUAL( 0, platformMock_DisplayFaultCauseToGdbConsoleCalls() );
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+"), platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 1, platformMock_GetLeavingDebuggerCalls() );
    CHECK_FALSE( IsFirstException() );

    // Clear mock state between mriDebugException(platformMock_GetContext()) calls.
    teardown();
    setup();

    platformMock_CommInitReceiveChecksummedData("+$c#");
        mriDebugException(platformMock_GetContext());
    CHECK_EQUAL( 1, platformMock_DisplayFaultCauseToGdbConsoleCalls() );
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+"), platformMock_CommGetTransmittedData() );
}

TEST(Mri, mriDebugException_SentStatusAndContinueCommands)
{
    mriInit("MRI_UART_MBED_USB");
    platformMock_CommInitReceiveChecksummedData("+$?#", "+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#" "+$T05responseT#" "+"),
                   platformMock_CommGetTransmittedData() );
}

TEST(Mri, mriDebugException_WhenSentInvalidCommand_ReturnsEmptyPacketResponse)
{
    mriInit("MRI_UART_MBED_USB");
    platformMock_CommInitReceiveChecksummedData("+$*#", "+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#" "+$#" "+"), platformMock_CommGetTransmittedData() );
}


TEST(Mri, mriDebugException_PacketBufferTooSmallShouldResultInBufferOverrunError)
{
    mriInit("MRI_UART_MBED_USB");
    platformMock_CommInitReceiveChecksummedData("+$?#", "+$c#");
    platformMock_SetPacketBufferSize(11);
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$" MRI_ERROR_BUFFER_OVERRUN "#"
                                                 "+$" MRI_ERROR_BUFFER_OVERRUN "#" "+"),
                                                 platformMock_CommGetTransmittedData() );
}




TEST(Mri, mriCoreSetTempBreakpoint_ShouldSucceedAndSetHardwareBreakpointAtSpecifiedAddressWithThumbBitCleared)
{
    mriInit("MRI_UART_MBED_USB");

    CHECK_TRUE( mriCore_SetTempBreakpoint(0xBAADF00D, NULL, NULL) );
    CHECK_EQUAL( 1, platformMock_SetHardwareBreakpointCalls() );
    CHECK_EQUAL( 0xBAADF00D & ~1, platformMock_SetHardwareBreakpointAddressArg() );
    CHECK_EQUAL( 0xFFFFFFFF, platformMock_SetHardwareBreakpointKindArg() );
}

TEST(Mri, mriCoreSetTempBreakpoint_TryToSetBreakpointTwiceAndSecondCallFails)
{
    mriInit("MRI_UART_MBED_USB");

    CHECK_TRUE( mriCore_SetTempBreakpoint(0xBAADF00D, NULL, NULL) );
    CHECK_EQUAL( 1, platformMock_SetHardwareBreakpointCalls() );
    CHECK_EQUAL( 0xBAADF00D & ~1, platformMock_SetHardwareBreakpointAddressArg() );
    CHECK_EQUAL( 0xFFFFFFFF, platformMock_SetHardwareBreakpointKindArg() );

    CHECK_FALSE( mriCore_SetTempBreakpoint(0xBAADF00D, NULL, NULL) );
    // Should still just have hardware breakpoint from first successful call.
    CHECK_EQUAL( 1, platformMock_SetHardwareBreakpointCalls() );
    CHECK_EQUAL( 0xBAADF00D & ~1, platformMock_SetHardwareBreakpointAddressArg() );
    CHECK_EQUAL( 0xFFFFFFFF, platformMock_SetHardwareBreakpointKindArg() );
}

TEST(Mri, mriCoreSetTempBreakpoint_SetHardwareBreakpointThrows_ShouldFail_ExceptionShouldBeCleared)
{
    mriInit("MRI_UART_MBED_USB");

    platformMock_SetHardwareBreakpointException(exceededHardwareResourcesException);
    CHECK_FALSE( mriCore_SetTempBreakpoint(0xBAADF00D, NULL, NULL) );
    CHECK_EQUAL( noException, getExceptionCode() );
}

TEST(Mri, mriCoreSetTempBreakpoint_RunMriDebugException_DontHitBreakpoint_BreakpointShouldNotBeCleared)
{
    mriInit("MRI_UART_MBED_USB");

    mriCore_SetTempBreakpoint(0xBAADF00D, NULL, NULL);

    mriPlatform_SetProgramCounter(0xBAADF00B);
    platformMock_CommInitReceiveChecksummedData("+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+"), platformMock_CommGetTransmittedData() );

    CHECK_EQUAL( 0xBAADF00D & ~1, platformMock_SetHardwareBreakpointAddressArg() );
    CHECK_EQUAL( 0, platformMock_ClearHardwareBreakpointCalls() );
}

TEST(Mri, mriCoreSetTempBreakpoint_RunMriDebugException_HitBreakpoint_BreakpointShouldBeCleared)
{
    mriInit("MRI_UART_MBED_USB");

    mriCore_SetTempBreakpoint(0xBAADF00D, NULL, NULL);

    mriPlatform_SetProgramCounter(0xBAADF00D);
    platformMock_CommInitReceiveChecksummedData("+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+"), platformMock_CommGetTransmittedData() );

    CHECK_EQUAL( 1, platformMock_ClearHardwareBreakpointCalls() );
}

TEST(Mri, mriCoreSetTempBreakpoint_RunMriDebugException_HitBreakpoint_ClearBreakpointThrows_BreakpointShouldStillBeCleared)
{
    mriInit("MRI_UART_MBED_USB");

    mriCore_SetTempBreakpoint(0xBAADF00D, NULL, NULL);

    mriPlatform_SetProgramCounter(0xBAADF00D);
    platformMock_ClearHardwareBreakpointException(memFaultException);
    platformMock_CommInitReceiveChecksummedData("+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+"), platformMock_CommGetTransmittedData() );

    CHECK_EQUAL( 1, platformMock_ClearHardwareBreakpointCalls() );
}

static int testCallback(void* pvContext){
    g_callbackCalled = true;
    g_pvCallbackContext = pvContext;
    return g_callbackReturnValue;
}

TEST(Mri, mriCoreSetTempBreakpoint_RunMriDebugException_WithCallbackReturning0_ShouldContinueToCommandParser)
{
    mriInit("MRI_UART_MBED_USB");

    mriCore_SetTempBreakpoint(0xBAADF00D, testCallback, (void*)this);

    g_callbackReturnValue = 0;
    mriPlatform_SetProgramCounter(0xBAADF00D);
    platformMock_CommInitReceiveChecksummedData("+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+"), platformMock_CommGetTransmittedData() );

    CHECK_EQUAL( 1, platformMock_ClearHardwareBreakpointCalls() );
    CHECK_TRUE( g_callbackCalled );
    POINTERS_EQUAL( this, g_pvCallbackContext );
}

TEST(Mri, mriCoreSetTempBreakpoint_RunMriDebugException_WithCallbackReturning1_ShouldReturnImmediately)
{
    mriInit("MRI_UART_MBED_USB");

    mriCore_SetTempBreakpoint(0xBAADF00D, testCallback, NULL);

    g_callbackReturnValue = 1;
    mriPlatform_SetProgramCounter(0xBAADF00D);
    platformMock_CommInitReceiveChecksummedData("+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData(""), platformMock_CommGetTransmittedData() );

    CHECK_EQUAL( 1, platformMock_ClearHardwareBreakpointCalls() );
    CHECK_TRUE( g_callbackCalled );
    POINTERS_EQUAL( NULL, g_pvCallbackContext );
}




struct HookCounts
{
    uint32_t enteringCount;
    uint32_t leavingCount;
};

static void enteringHook(void* pvContext)
{
    HookCounts* pCounts = (HookCounts*)pvContext;
    pCounts->enteringCount++;
}

static void leavingHook(void* pvContext)
{
    HookCounts* pCounts = (HookCounts*)pvContext;
    pCounts->leavingCount++;
}

TEST(Mri, mriCoreSetDebuggerHooks_HookBothFunctions)
{
    HookCounts counts = { .enteringCount = 0, .leavingCount = 0 };

    mriInit("MRI_UART_MBED_USB");

    mriCoreSetDebuggerHooks(enteringHook, leavingHook, &counts);

    platformMock_CommInitReceiveChecksummedData("+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+"), platformMock_CommGetTransmittedData() );

    LONGS_EQUAL(1, counts.enteringCount);
    LONGS_EQUAL(1, counts.leavingCount);
}

TEST(Mri, mriCoreSetDebuggerHooks_HookEnteringFunctionOnly)
{
    HookCounts counts = { .enteringCount = 0, .leavingCount = 0 };

    mriInit("MRI_UART_MBED_USB");

    mriCoreSetDebuggerHooks(enteringHook, NULL, &counts);

    platformMock_CommInitReceiveChecksummedData("+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+"), platformMock_CommGetTransmittedData() );

    LONGS_EQUAL(1, counts.enteringCount);
    LONGS_EQUAL(0, counts.leavingCount);
}

TEST(Mri, mriCoreSetDebuggerHooks_HookLeavingFunctionOnly)
{
    HookCounts counts = { .enteringCount = 0, .leavingCount = 0 };

    mriInit("MRI_UART_MBED_USB");

    mriCoreSetDebuggerHooks(NULL, leavingHook, &counts);

    platformMock_CommInitReceiveChecksummedData("+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+"), platformMock_CommGetTransmittedData() );

    LONGS_EQUAL(0, counts.enteringCount);
    LONGS_EQUAL(1, counts.leavingCount);
}
