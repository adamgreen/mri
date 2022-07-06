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
#include <core/platforms.h>
}
#include <platformMock.h>

// Include C++ headers for test harness.
#include "CppUTest/TestHarness.h"


TEST_GROUP(cmdStep)
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

TEST(cmdStep, BasicSingleStep)
{
    CHECK_FALSE ( Platform_IsSingleStepping() );
    platformMock_CommInitReceiveChecksummedData("+$s#");
        mriDebugException(platformMock_GetContext());
    CHECK_TRUE ( Platform_IsSingleStepping() );
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+"), platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 0, platformMock_AdvanceProgramCounterToNextInstructionCalls() );
    CHECK_EQUAL( INITIAL_PC, platformMock_GetProgramCounterValue() );
}

TEST(cmdStep, SingleStepOverHardcodedBreakpoints_MustContinueAfterToExit)
{
    platformMock_SetTypeOfCurrentInstruction(MRI_PLATFORM_INSTRUCTION_HARDCODED_BREAKPOINT);
    platformMock_CommInitReceiveChecksummedData("+$s#", "+$c#");
        mriDebugException(platformMock_GetContext());
    CHECK_FALSE ( Platform_IsSingleStepping() );
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$T05responseT#+"),
                   platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 2, platformMock_AdvanceProgramCounterToNextInstructionCalls() );
    CHECK_EQUAL( INITIAL_PC + 8, platformMock_GetProgramCounterValue() );
}

TEST(cmdStep, SingleStep_AdvancePCLikeSkippingOverBasePriInstructions_MustContinueAfterToExit)
{
    platformMock_SingleStepShouldAdvancePC(true);
    platformMock_CommInitReceiveChecksummedData("+$s#", "+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$T05responseT#+"),
                   platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( INITIAL_PC + 4, platformMock_GetProgramCounterValue() );
}

TEST(cmdStep, SetSignalOnly)
{
    CHECK_FALSE ( Platform_IsSingleStepping() );
    platformMock_CommInitReceiveChecksummedData("+$S0b#");
        mriDebugException(platformMock_GetContext());
    CHECK_TRUE ( Platform_IsSingleStepping() );
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+"), platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 0, platformMock_AdvanceProgramCounterToNextInstructionCalls() );
    CHECK_EQUAL( INITIAL_PC, platformMock_GetProgramCounterValue() );
}

TEST(cmdStep, SetSignalWithAddress)
{
    CHECK_FALSE ( Platform_IsSingleStepping() );
    platformMock_CommInitReceiveChecksummedData("+$S0b;f00d#");
        mriDebugException(platformMock_GetContext());
    CHECK_TRUE ( Platform_IsSingleStepping() );
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+"), platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 0, platformMock_AdvanceProgramCounterToNextInstructionCalls() );
    CHECK_EQUAL( 0xF00D, platformMock_GetProgramCounterValue() );
}

TEST(cmdStep, SetSignalSingleStepOverHardcodedBreakpoints_MustContinueAfterToExit)
{
    platformMock_SetTypeOfCurrentInstruction(MRI_PLATFORM_INSTRUCTION_HARDCODED_BREAKPOINT);
    platformMock_CommInitReceiveChecksummedData("+$S0b#", "+$c#");
        mriDebugException(platformMock_GetContext());
    CHECK_FALSE ( Platform_IsSingleStepping() );
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$T05responseT#+"),
                   platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 2, platformMock_AdvanceProgramCounterToNextInstructionCalls() );
    CHECK_EQUAL( INITIAL_PC + 8, platformMock_GetProgramCounterValue() );
}

TEST(cmdStep, SetSignalWithMissingSignalValue)
{
    CHECK_FALSE ( Platform_IsSingleStepping() );
    platformMock_CommInitReceiveChecksummedData("+$S#", "+$c#");
        mriDebugException(platformMock_GetContext());
    CHECK_FALSE ( Platform_IsSingleStepping() );
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$" MRI_ERROR_INVALID_ARGUMENT "#+"),
                  platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 0, platformMock_AdvanceProgramCounterToNextInstructionCalls() );
    CHECK_EQUAL( INITIAL_PC, platformMock_GetProgramCounterValue() );
}

TEST(cmdStep, SetSignalCommandWithMissingAddressAfterSemicolon)
{
    CHECK_FALSE ( Platform_IsSingleStepping() );
    platformMock_CommInitReceiveChecksummedData("+$S0b;#", "+$c#");
        mriDebugException(platformMock_GetContext());
    CHECK_FALSE ( Platform_IsSingleStepping() );
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$" MRI_ERROR_INVALID_ARGUMENT "#+"),
                   platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 0, platformMock_AdvanceProgramCounterToNextInstructionCalls() );
    CHECK_EQUAL( INITIAL_PC, platformMock_GetProgramCounterValue() );
}

TEST(cmdStep, RtosSetThreadStateEnabled_BasicSingleStep_VerifyThreadStatesAreSet)
{
    PlatformMockThread threads[3];
    threads[0].threadId = 0x5A5A5A5A;
    threads[0].state = MRI_PLATFORM_THREAD_FROZEN;
    threads[1].threadId = 0xBAADF00D;
    threads[1].state = MRI_PLATFORM_THREAD_FROZEN;
    threads[2].threadId = 0xBAADFEED;
    threads[2].state = MRI_PLATFORM_THREAD_FROZEN;
    platformMock_RtosSetThreadList(threads, sizeof(threads)/sizeof(threads[0]));
    platformMock_RtosSetHaltedThreadId(0xBAADFEED);
    platformMock_RtosSetIsSetThreadStateSupported(1);
    CHECK_FALSE ( Platform_IsSingleStepping() );
    platformMock_CommInitReceiveChecksummedData("+$s#");
        mriDebugException(platformMock_GetContext());
    CHECK_TRUE ( Platform_IsSingleStepping() );
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05thread:baadfeed;responseT#+"), platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 0, platformMock_AdvanceProgramCounterToNextInstructionCalls() );
    CHECK_EQUAL( INITIAL_PC, platformMock_GetProgramCounterValue() );
    CHECK_EQUAL( MRI_PLATFORM_THREAD_THAWED, threads[0].state );
    CHECK_EQUAL( MRI_PLATFORM_THREAD_THAWED, threads[1].state );
    CHECK_EQUAL( MRI_PLATFORM_THREAD_SINGLE_STEPPING, threads[2].state );
}

TEST(cmdStep, RtosSetThreadStateEnabled_SetSignalOnly_VerifyThreadStatesAreSet)
{
    PlatformMockThread threads[3];
    threads[0].threadId = 0x5A5A5A5A;
    threads[0].state = MRI_PLATFORM_THREAD_FROZEN;
    threads[1].threadId = 0xBAADF00D;
    threads[1].state = MRI_PLATFORM_THREAD_FROZEN;
    threads[2].threadId = 0xBAADFEED;
    threads[2].state = MRI_PLATFORM_THREAD_FROZEN;
    platformMock_RtosSetThreadList(threads, sizeof(threads)/sizeof(threads[0]));
    platformMock_RtosSetHaltedThreadId(0xBAADFEED);
    platformMock_RtosSetIsSetThreadStateSupported(1);
    CHECK_FALSE ( Platform_IsSingleStepping() );
    platformMock_CommInitReceiveChecksummedData("+$S0b#");
        mriDebugException(platformMock_GetContext());
    CHECK_TRUE ( Platform_IsSingleStepping() );
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05thread:baadfeed;responseT#+"), platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 0, platformMock_AdvanceProgramCounterToNextInstructionCalls() );
    CHECK_EQUAL( INITIAL_PC, platformMock_GetProgramCounterValue() );
    CHECK_EQUAL( MRI_PLATFORM_THREAD_THAWED, threads[0].state );
    CHECK_EQUAL( MRI_PLATFORM_THREAD_THAWED, threads[1].state );
    CHECK_EQUAL( MRI_PLATFORM_THREAD_SINGLE_STEPPING, threads[2].state );
}
