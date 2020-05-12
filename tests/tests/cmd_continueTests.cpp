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
}
#include <platformMock.h>

// Include C++ headers for test harness.
#include "CppUTest/TestHarness.h"


TEST_GROUP(cmdContinue)
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

TEST(cmdContinue, SkipOverHardcodedBreakpoints)
{
    platformMock_SetTypeOfCurrentInstruction(MRI_PLATFORM_INSTRUCTION_HARDCODED_BREAKPOINT);
    platformMock_CommInitReceiveChecksummedData("+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+"), platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 1, platformMock_AdvanceProgramCounterToNextInstructionCalls() );
    CHECK_EQUAL( INITIAL_PC + 4, platformMock_GetProgramCounterValue() );
}

TEST(cmdContinue, SetProgramCounterWithContinueCommand)
{
    platformMock_CommInitReceiveChecksummedData("+$cf00d#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+"), platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 0, platformMock_AdvanceProgramCounterToNextInstructionCalls() );
    CHECK_EQUAL( 0xF00D, platformMock_GetProgramCounterValue() );
}

TEST(cmdContinue, SetSignalOnly)
{
    platformMock_CommInitReceiveChecksummedData("+$C0b#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+"), platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 0, platformMock_AdvanceProgramCounterToNextInstructionCalls() );
    CHECK_EQUAL( INITIAL_PC, platformMock_GetProgramCounterValue() );
}

TEST(cmdContinue, SetSignalWithAddress)
{
    platformMock_CommInitReceiveChecksummedData("+$C0b;f00d#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+"), platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 0, platformMock_AdvanceProgramCounterToNextInstructionCalls() );
    CHECK_EQUAL( 0xF00D, platformMock_GetProgramCounterValue() );
}

TEST(cmdContinue, SetSignalSkipOverHardcodedBreakpoints)
{
    platformMock_SetTypeOfCurrentInstruction(MRI_PLATFORM_INSTRUCTION_HARDCODED_BREAKPOINT);
    platformMock_CommInitReceiveChecksummedData("+$C0b#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+"), platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 1, platformMock_AdvanceProgramCounterToNextInstructionCalls() );
    CHECK_EQUAL( INITIAL_PC + 4, platformMock_GetProgramCounterValue() );
}

TEST(cmdContinue, SetSignalCommandWithMissingSignalValue)
{
    platformMock_CommInitReceiveChecksummedData("+$C#", "+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$" MRI_ERROR_INVALID_ARGUMENT "#+"),
                   platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 0, platformMock_AdvanceProgramCounterToNextInstructionCalls() );
    CHECK_EQUAL( INITIAL_PC, platformMock_GetProgramCounterValue() );
}

TEST(cmdContinue, SetSignalCommandWithMissingAddressAfterSemicolon)
{
    platformMock_CommInitReceiveChecksummedData("+$C0b;#", "+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$" MRI_ERROR_INVALID_ARGUMENT "#+"),
                   platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 0, platformMock_AdvanceProgramCounterToNextInstructionCalls() );
    CHECK_EQUAL( INITIAL_PC, platformMock_GetProgramCounterValue() );
}

TEST(cmdContinue, RtosSetThreadStateEnabled_SkipOverHardcodedBreakpoints_VerifyThreadStatesAreSet)
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
    platformMock_SetTypeOfCurrentInstruction(MRI_PLATFORM_INSTRUCTION_HARDCODED_BREAKPOINT);
    platformMock_CommInitReceiveChecksummedData("+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05thread:baadfeed;responseT#+"), platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 1, platformMock_AdvanceProgramCounterToNextInstructionCalls() );
    CHECK_EQUAL( INITIAL_PC + 4, platformMock_GetProgramCounterValue() );
    CHECK_EQUAL( MRI_PLATFORM_THREAD_THAWED, threads[0].state );
    CHECK_EQUAL( MRI_PLATFORM_THREAD_THAWED, threads[1].state );
    CHECK_EQUAL( MRI_PLATFORM_THREAD_THAWED, threads[2].state );
}

TEST(cmdContinue, DetachCommand_ShouldWorkSimilarToContinueCommand_ButReturnOk)
{
    platformMock_CommInitReceiveChecksummedData("+$D#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$OK#"), platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 0, platformMock_AdvanceProgramCounterToNextInstructionCalls() );
    CHECK_EQUAL( INITIAL_PC, platformMock_GetProgramCounterValue() );
}

TEST(cmdContinue, DetachCommand_ShouldWorkSimilarToContinueCommand_SkipOverHardcodedBreakpoints)
{
    platformMock_SetTypeOfCurrentInstruction(MRI_PLATFORM_INSTRUCTION_HARDCODED_BREAKPOINT);
    platformMock_CommInitReceiveChecksummedData("+$D#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$OK#"), platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 1, platformMock_AdvanceProgramCounterToNextInstructionCalls() );
    CHECK_EQUAL( INITIAL_PC + 4, platformMock_GetProgramCounterValue() );
}

TEST(cmdContinue, RtosSetThreadStateEnabled_DetachCommand_VerifyThreadStatesAreSet)
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
    platformMock_CommInitReceiveChecksummedData("+$D#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05thread:baadfeed;responseT#+$OK#"), platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 0, platformMock_AdvanceProgramCounterToNextInstructionCalls() );
    CHECK_EQUAL( INITIAL_PC, platformMock_GetProgramCounterValue() );
    CHECK_EQUAL( MRI_PLATFORM_THREAD_THAWED, threads[0].state );
    CHECK_EQUAL( MRI_PLATFORM_THREAD_THAWED, threads[1].state );
    CHECK_EQUAL( MRI_PLATFORM_THREAD_THAWED, threads[2].state );
}
