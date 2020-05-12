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
#include <core/platforms.h>
}
#include <platformMock.h>

// Include C++ headers for test harness.
#include "CppUTest/TestHarness.h"

#define UNTOUCHED_STATE ((PlatformThreadState)-1)


TEST_GROUP(cmdVCont)
{
    enum { THREAD_COUNT = 3 };

    Token              m_token;
    PlatformMockThread m_threads[THREAD_COUNT];
    uint32_t           m_expectedInvalidAttempts;
    int                m_expectedException;

    void setup()
    {
        m_expectedException = noException;
        m_threads[0].threadId = 0x5A5A5A5A;
        m_threads[0].state = MRI_PLATFORM_THREAD_FROZEN;
        m_threads[1].threadId = 0xBAADF00D;
        m_threads[1].state = MRI_PLATFORM_THREAD_FROZEN;
        m_threads[2].threadId = 0xBAADFEED;
        m_threads[2].state = MRI_PLATFORM_THREAD_FROZEN;
        m_expectedInvalidAttempts = 0;
        platformMock_Init();
        platformMock_RtosSetThreadList(m_threads, THREAD_COUNT);
        mriInit("MRI_UART_MBED_USB");
    }

    void teardown()
    {
        LONGS_EQUAL( m_expectedInvalidAttempts, platformMock_RtosGetThreadStateInvalidAttempts() );
        LONGS_EQUAL( m_expectedException, getExceptionCode() );
        clearExceptionCode();
        platformMock_Uninit();
    }

    void validateExceptionCode(int expectedExceptionCode)
    {
        m_expectedException = expectedExceptionCode;
        LONGS_EQUAL ( expectedExceptionCode, getExceptionCode() );
    }
};


TEST(cmdVCont, UnknownCommandStartingWithV_ShouldReturnEmptyResponse)
{
    platformMock_CommInitReceiveChecksummedData("+$vFoo#", "+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$#+"), platformMock_CommGetTransmittedData() );
}



// vCont? Tests
TEST(cmdVCont, vContQst_VerifyOutput)
{
    platformMock_CommInitReceiveChecksummedData("+$vCont?#", "+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$vCont;c;C;s;S;r#+"), platformMock_CommGetTransmittedData() );
}



// vCont Tests
TEST(cmdVCont, vCont_MissingSemicolonBeforeAction_ShouldReturnError)
{
    platformMock_CommInitReceiveChecksummedData("+$vCont:c#", "+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$" MRI_ERROR_INVALID_ARGUMENT "#+"),
                   platformMock_CommGetTransmittedData() );
}

TEST(cmdVCont, vCont_MissingActions_ShouldReturnError)
{
    platformMock_CommInitReceiveChecksummedData("+$vCont#", "+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$" MRI_ERROR_INVALID_ARGUMENT "#+"),
                   platformMock_CommGetTransmittedData() );
}

TEST(cmdVCont, vCont_UnknownAction_ShouldReturnError)
{
    platformMock_CommInitReceiveChecksummedData("+$vCont;X#", "+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$" MRI_ERROR_INVALID_ARGUMENT "#+"),
                   platformMock_CommGetTransmittedData() );
}

TEST(cmdVCont, vCont_ContinueDefaultAction_SkipOverHardcodedBreakpoint)
{
    platformMock_SetTypeOfCurrentInstruction(MRI_PLATFORM_INSTRUCTION_HARDCODED_BREAKPOINT);
    platformMock_CommInitReceiveChecksummedData("+$vCont;c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+"), platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 1, platformMock_AdvanceProgramCounterToNextInstructionCalls() );
    CHECK_EQUAL( INITIAL_PC + 4, platformMock_GetProgramCounterValue() );
}

TEST(cmdVCont, vCont_ContinueAllThreads_SkipOverHardcodedBreakpoint)
{
    platformMock_SetTypeOfCurrentInstruction(MRI_PLATFORM_INSTRUCTION_HARDCODED_BREAKPOINT);
    platformMock_CommInitReceiveChecksummedData("+$vCont;c:-1#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+"), platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 1, platformMock_AdvanceProgramCounterToNextInstructionCalls() );
    CHECK_EQUAL( INITIAL_PC + 4, platformMock_GetProgramCounterValue() );
}

TEST(cmdVCont, vCont_ContinueWithSignalDefaultAction_SkipOverHardcodedBreakpoint)
{
    platformMock_SetTypeOfCurrentInstruction(MRI_PLATFORM_INSTRUCTION_HARDCODED_BREAKPOINT);
    platformMock_CommInitReceiveChecksummedData("+$vCont;Cab#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+"), platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 1, platformMock_AdvanceProgramCounterToNextInstructionCalls() );
    CHECK_EQUAL( INITIAL_PC + 4, platformMock_GetProgramCounterValue() );
}

TEST(cmdVCont, vCont_ContinueWithSignalAllThreads_SkipOverHardcodedBreakpoint)
{
    platformMock_SetTypeOfCurrentInstruction(MRI_PLATFORM_INSTRUCTION_HARDCODED_BREAKPOINT);
    platformMock_CommInitReceiveChecksummedData("+$vCont;Cab:-1#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+"), platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 1, platformMock_AdvanceProgramCounterToNextInstructionCalls() );
    CHECK_EQUAL( INITIAL_PC + 4, platformMock_GetProgramCounterValue() );
}

TEST(cmdVCont, vCont_MultipleContinueActionsOfVariousTypes_SkipOverHardcodedBreakpoint)
{
    platformMock_RtosSetHaltedThreadId(0xBAADF00D);
    platformMock_SetTypeOfCurrentInstruction(MRI_PLATFORM_INSTRUCTION_HARDCODED_BREAKPOINT);
    platformMock_CommInitReceiveChecksummedData("+$vCont;c;Cab:-1;c:baadfeed;c:baadf00d#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05thread:baadf00d;responseT#+"), platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 1, platformMock_AdvanceProgramCounterToNextInstructionCalls() );
    CHECK_EQUAL( INITIAL_PC + 4, platformMock_GetProgramCounterValue() );
}

TEST(cmdVCont, vCont_ContinueWithMissingSignalValue_ShouldReturnError)
{
    platformMock_SetTypeOfCurrentInstruction(MRI_PLATFORM_INSTRUCTION_HARDCODED_BREAKPOINT);
    platformMock_CommInitReceiveChecksummedData("+$vCont;C#", "+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$" MRI_ERROR_INVALID_ARGUMENT "#+"),
                   platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 1, platformMock_AdvanceProgramCounterToNextInstructionCalls() );
    CHECK_EQUAL( INITIAL_PC + 4, platformMock_GetProgramCounterValue() );
}

TEST(cmdVCont, vCont_SingleStepActionOnly)
{
    CHECK_FALSE ( Platform_IsSingleStepping() );
    platformMock_CommInitReceiveChecksummedData("+$vCont;s#");
        mriDebugException(platformMock_GetContext());
    CHECK_TRUE ( Platform_IsSingleStepping() );
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+"), platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 0, platformMock_AdvanceProgramCounterToNextInstructionCalls() );
    CHECK_EQUAL( INITIAL_PC, platformMock_GetProgramCounterValue() );
}

TEST(cmdVCont, vCont_SingleStepAllThreads)
{
    CHECK_FALSE ( Platform_IsSingleStepping() );
    platformMock_CommInitReceiveChecksummedData("+$vCont;s:-1#");
        mriDebugException(platformMock_GetContext());
    CHECK_TRUE ( Platform_IsSingleStepping() );
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+"), platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 0, platformMock_AdvanceProgramCounterToNextInstructionCalls() );
    CHECK_EQUAL( INITIAL_PC, platformMock_GetProgramCounterValue() );
}

TEST(cmdVCont, vCont_SingleStepAllThreadsAndDefaultOfContinue)
{
    CHECK_FALSE ( Platform_IsSingleStepping() );
    platformMock_CommInitReceiveChecksummedData("+$vCont;s:-1;c#");
        mriDebugException(platformMock_GetContext());
    CHECK_TRUE ( Platform_IsSingleStepping() );
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+"), platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 0, platformMock_AdvanceProgramCounterToNextInstructionCalls() );
    CHECK_EQUAL( INITIAL_PC, platformMock_GetProgramCounterValue() );
}

TEST(cmdVCont, vCont_SingleStepHaltedThreadAndDefaultOfContinue)
{
    platformMock_RtosSetHaltedThreadId(0xBAADF00D);
    CHECK_FALSE ( Platform_IsSingleStepping() );
    platformMock_CommInitReceiveChecksummedData("+$vCont;s:baadf00d;c#");
        mriDebugException(platformMock_GetContext());
    CHECK_TRUE ( Platform_IsSingleStepping() );
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05thread:baadf00d;responseT#+"), platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 0, platformMock_AdvanceProgramCounterToNextInstructionCalls() );
    CHECK_EQUAL( INITIAL_PC, platformMock_GetProgramCounterValue() );
}

TEST(cmdVCont, vCont_SingleStepWithSignalValueActionOnly)
{
    CHECK_FALSE ( Platform_IsSingleStepping() );
    platformMock_CommInitReceiveChecksummedData("+$vCont;Sab#");
        mriDebugException(platformMock_GetContext());
    CHECK_TRUE ( Platform_IsSingleStepping() );
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+"), platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 0, platformMock_AdvanceProgramCounterToNextInstructionCalls() );
    CHECK_EQUAL( INITIAL_PC, platformMock_GetProgramCounterValue() );
}

TEST(cmdVCont, vCont_SingleStepWithSignalValueAllThreads)
{
    CHECK_FALSE ( Platform_IsSingleStepping() );
    platformMock_CommInitReceiveChecksummedData("+$vCont;Sab:-1#");
        mriDebugException(platformMock_GetContext());
    CHECK_TRUE ( Platform_IsSingleStepping() );
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+"), platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 0, platformMock_AdvanceProgramCounterToNextInstructionCalls() );
    CHECK_EQUAL( INITIAL_PC, platformMock_GetProgramCounterValue() );
}

TEST(cmdVCont, vCont_MultipleSingleStepActionsOfVariousTypes)
{
    platformMock_RtosSetHaltedThreadId(0xBAADF00D);
    platformMock_CommInitReceiveChecksummedData("+$vCont;s;Sab:-1;s:baadfeed;s:baadf00d#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05thread:baadf00d;responseT#+"), platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 0, platformMock_AdvanceProgramCounterToNextInstructionCalls() );
    CHECK_EQUAL( INITIAL_PC, platformMock_GetProgramCounterValue() );
}

TEST(cmdVCont, vCont_MultipleSingleStepWithRangedSingleStepActionsOfVariousTypes)
{
    platformMock_RtosSetHaltedThreadId(0xBAADF00D);
    platformMock_CommInitReceiveChecksummedData("+$vCont;s;Sab:-1;s:baadfeed;r10000000,10000004:baadf00d#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05thread:baadf00d;responseT#+"), platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 0, platformMock_AdvanceProgramCounterToNextInstructionCalls() );
    CHECK_EQUAL( INITIAL_PC, platformMock_GetProgramCounterValue() );
}

TEST(cmdVCont, vCont_SingleStepWithMissingSignalValue_ShouldReturnError)
{
    platformMock_CommInitReceiveChecksummedData("+$vCont;S#", "+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$" MRI_ERROR_INVALID_ARGUMENT "#+"),
                   platformMock_CommGetTransmittedData() );
}

TEST(cmdVCont, vCont_RangedSingleStepActionOnly)
{
    CHECK_FALSE ( Platform_IsSingleStepping() );
    platformMock_CommInitReceiveChecksummedData("+$vCont;r10000000,10000004#");
        mriDebugException(platformMock_GetContext());
    CHECK_TRUE ( Platform_IsSingleStepping() );
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+"), platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 0, platformMock_AdvanceProgramCounterToNextInstructionCalls() );
    CHECK_EQUAL( INITIAL_PC, platformMock_GetProgramCounterValue() );
}

TEST(cmdVCont, vCont_RangedSingleStepAllThreadsAndContinueAsDefault)
{
    CHECK_FALSE ( Platform_IsSingleStepping() );
    platformMock_CommInitReceiveChecksummedData("+$vCont;r10000000,10000004:-1;c#");
        mriDebugException(platformMock_GetContext());
    CHECK_TRUE ( Platform_IsSingleStepping() );
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+"), platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 0, platformMock_AdvanceProgramCounterToNextInstructionCalls() );
    CHECK_EQUAL( INITIAL_PC, platformMock_GetProgramCounterValue() );
}

TEST(cmdVCont, vCont_RangedSingleStepHaltedThreadAndContinueAsDefault)
{
    platformMock_RtosSetHaltedThreadId(0xBAADF00D);
    CHECK_FALSE ( Platform_IsSingleStepping() );
    platformMock_CommInitReceiveChecksummedData("+$vCont;r10000000,10000004:baadf00d;c#");
        mriDebugException(platformMock_GetContext());
    CHECK_TRUE ( Platform_IsSingleStepping() );
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05thread:baadf00d;responseT#+"), platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 0, platformMock_AdvanceProgramCounterToNextInstructionCalls() );
    CHECK_EQUAL( INITIAL_PC, platformMock_GetProgramCounterValue() );
}

TEST(cmdVCont, vCont_RangedSingleStep_VerifyOnlySingleStepsInDesiredRange)
{
    CHECK_FALSE ( Platform_IsSingleStepping() );
    platformMock_CommInitReceiveChecksummedData("+$vCont;r10000000,10000004#");
        mriDebugException(platformMock_GetContext());
    CHECK_TRUE ( Platform_IsSingleStepping() );
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+"), platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 0, platformMock_AdvanceProgramCounterToNextInstructionCalls() );
    CHECK_EQUAL( INITIAL_PC, platformMock_GetProgramCounterValue() );

    // This stop is still in single step range so should just return.
    Platform_SetProgramCounter(0x10000000);
    platformMock_CommInitTransmitDataBuffer(512);
    platformMock_CommInitReceiveChecksummedData("");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData(""), platformMock_CommGetTransmittedData() );

    // This stop is still in single step range so should just return.
    Platform_SetProgramCounter(0x10000002);
    platformMock_CommInitTransmitDataBuffer(512);
    platformMock_CommInitReceiveChecksummedData("");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData(""), platformMock_CommGetTransmittedData() );

    // This stop is out of the single step range so it should send T stop response.
    Platform_SetProgramCounter(0x10000004);
    platformMock_CommInitTransmitDataBuffer(512);
    platformMock_CommInitReceiveChecksummedData("+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+"), platformMock_CommGetTransmittedData() );
}

TEST(cmdVCont, vCont_SingleStepAllWithHigherPriorityRangedSingleStep_VerifyOnlySingleStepsInDesiredRange)
{
    CHECK_FALSE ( Platform_IsSingleStepping() );
    platformMock_CommInitReceiveChecksummedData("+$vCont;s;r10000000,10000004:-1#");
        mriDebugException(platformMock_GetContext());
    CHECK_TRUE ( Platform_IsSingleStepping() );
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+"), platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 0, platformMock_AdvanceProgramCounterToNextInstructionCalls() );
    CHECK_EQUAL( INITIAL_PC, platformMock_GetProgramCounterValue() );

    // This stop is still in single step range so should just return.
    Platform_SetProgramCounter(0x10000000);
    platformMock_CommInitTransmitDataBuffer(512);
    platformMock_CommInitReceiveChecksummedData("");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData(""), platformMock_CommGetTransmittedData() );

    // This stop is still in single step range so should just return.
    Platform_SetProgramCounter(0x10000002);
    platformMock_CommInitTransmitDataBuffer(512);
    platformMock_CommInitReceiveChecksummedData("");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData(""), platformMock_CommGetTransmittedData() );

    // This stop is out of the single step range so it should send T stop response.
    Platform_SetProgramCounter(0x10000004);
    platformMock_CommInitTransmitDataBuffer(512);
    platformMock_CommInitReceiveChecksummedData("+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+"), platformMock_CommGetTransmittedData() );
}

TEST(cmdVCont, vCont_RangedSingleStep_VerifyCanBeInterruptedEvenWhenInRange)
{
    CHECK_FALSE ( Platform_IsSingleStepping() );
    platformMock_CommInitReceiveChecksummedData("+$vCont;r10000000,10000004#");
        mriDebugException(platformMock_GetContext());
    CHECK_TRUE ( Platform_IsSingleStepping() );
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+"), platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 0, platformMock_AdvanceProgramCounterToNextInstructionCalls() );
    CHECK_EQUAL( INITIAL_PC, platformMock_GetProgramCounterValue() );

    // This stop is still in single step range so should just return.
    Platform_SetProgramCounter(0x10000000);
    platformMock_CommInitTransmitDataBuffer(512);
    platformMock_CommInitReceiveChecksummedData("");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData(""), platformMock_CommGetTransmittedData() );

    // Simulate a CTRL+C interrupt as cause and should get T response instead.
    Platform_SetProgramCounter(0x10000002);
    platformMock_SetCauseOfException(SIGINT);
    platformMock_CommInitTransmitDataBuffer(512);
    platformMock_CommInitReceiveChecksummedData("+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T02responseT#+"), platformMock_CommGetTransmittedData() );
}

TEST(cmdVCont, vCont_RangedSingleStep_VerifyWillStopForBreakpointpointEvenWhenInRange)
{
    CHECK_FALSE ( Platform_IsSingleStepping() );
    platformMock_CommInitReceiveChecksummedData("+$vCont;r10000000,10000004#");
        mriDebugException(platformMock_GetContext());
    CHECK_TRUE ( Platform_IsSingleStepping() );
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+"), platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 0, platformMock_AdvanceProgramCounterToNextInstructionCalls() );
    CHECK_EQUAL( INITIAL_PC, platformMock_GetProgramCounterValue() );

    // This stop is still in single step range so should just return.
    Platform_SetProgramCounter(0x10000000);
    platformMock_CommInitTransmitDataBuffer(512);
    platformMock_CommInitReceiveChecksummedData("");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData(""), platformMock_CommGetTransmittedData() );

    // Simulate a hardware breakpoint as cause and should get T response instead.
    PlatformTrapReason breakpointReason = { MRI_PLATFORM_TRAP_TYPE_HWBREAK, 0x10000002 };
    Platform_SetProgramCounter(0x10000002);
    platformMock_SetTrapReason(&breakpointReason);
    platformMock_CommInitTransmitDataBuffer(512);
    platformMock_CommInitReceiveChecksummedData("+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+"), platformMock_CommGetTransmittedData() );
}

TEST(cmdVCont, vCont_RangedSingleStep_StepOverOneHardcodedBreakpoint_NeedToContinueOverAnotherBreakpointToExit)
{
    platformMock_SetTypeOfCurrentInstruction(MRI_PLATFORM_INSTRUCTION_HARDCODED_BREAKPOINT);
    platformMock_CommInitReceiveChecksummedData("+$vCont;r10000000,10000004#", "+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$T05responseT#+"), platformMock_CommGetTransmittedData() );
    // Called once for single stepping and another time for the continue used to return from mriDebugException.
    CHECK_EQUAL( 2, platformMock_AdvanceProgramCounterToNextInstructionCalls() );
    // Each instruction advance is hardcoded to 4 bytes in the mock.
    CHECK_EQUAL( 0x10000008, platformMock_GetProgramCounterValue() );
}

TEST(cmdVCont, vCont_RangedSingleStep_StepOverOneOfTwoHardcodedBreakpointsInRange_NeedToContinueOverAnotherBreakpointToExit)
{
    platformMock_SetTypeOfCurrentInstruction(MRI_PLATFORM_INSTRUCTION_HARDCODED_BREAKPOINT);
    platformMock_CommInitReceiveChecksummedData("+$vCont;r10000000,10000008#", "+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$T05responseT#+"), platformMock_CommGetTransmittedData() );
    // Called once for single stepping and another time for the continue used to return from mriDebugException.
    CHECK_EQUAL( 2, platformMock_AdvanceProgramCounterToNextInstructionCalls() );
    // Each instruction advance is hardcoded to 4 bytes in the mock.
    CHECK_EQUAL( 0x10000008, platformMock_GetProgramCounterValue() );
}

TEST(cmdVCont, vCont_RangedSingleStepWithMissingRange_ShouldReturnError)
{
    platformMock_CommInitReceiveChecksummedData("+$vCont;r#", "+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$" MRI_ERROR_INVALID_ARGUMENT "#+"),
                   platformMock_CommGetTransmittedData() );
}

TEST(cmdVCont, vCont_RangedSingleStepWithMissingComma_ShouldReturnError)
{
    platformMock_CommInitReceiveChecksummedData("+$vCont;r10000000#", "+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$" MRI_ERROR_INVALID_ARGUMENT "#+"),
                   platformMock_CommGetTransmittedData() );
}

TEST(cmdVCont, vCont_RangedSingleStepWithMissingEndValue_ShouldReturnError)
{
    platformMock_CommInitReceiveChecksummedData("+$vCont;r10000000,#", "+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$" MRI_ERROR_INVALID_ARGUMENT "#+"),
                   platformMock_CommGetTransmittedData() );
}

TEST(cmdVCont, vCont_SingleStepWithNonHaltedThreadId_ShouldBeTreatedLikeItWasHaltedThreadId)
{
    platformMock_RtosSetHaltedThreadId(0xBAADF00D);
    CHECK_FALSE ( Platform_IsSingleStepping() );
    platformMock_CommInitReceiveChecksummedData("+$vCont;s:baadfeed;c#");
        mriDebugException(platformMock_GetContext());
    CHECK_TRUE ( Platform_IsSingleStepping() );
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05thread:baadf00d;responseT#+"), platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 0, platformMock_AdvanceProgramCounterToNextInstructionCalls() );
    CHECK_EQUAL( INITIAL_PC, platformMock_GetProgramCounterValue() );
}

TEST(cmdVCont, vCont_RangedSingleStepWithNonHaltedThreadId_ShouldBeTreatedLikeItWasHaltedThreadId)
{
    platformMock_RtosSetHaltedThreadId(0xBAADF00D);
    CHECK_FALSE ( Platform_IsSingleStepping() );
    platformMock_CommInitReceiveChecksummedData("+$vCont;r10000000,10000004:baadfeed;c#");
        mriDebugException(platformMock_GetContext());
    CHECK_TRUE ( Platform_IsSingleStepping() );
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05thread:baadf00d;responseT#+"), platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 0, platformMock_AdvanceProgramCounterToNextInstructionCalls() );
    CHECK_EQUAL( INITIAL_PC, platformMock_GetProgramCounterValue() );
}

TEST(cmdVCont, vCont_RtosSetThreadStateEnabled_SingleStepHaltedThreadAndContinue)
{
    platformMock_RtosSetHaltedThreadId(0xBAADFEED);
    platformMock_RtosSetIsSetThreadStateSupported(1);
    CHECK_FALSE ( Platform_IsSingleStepping() );
    platformMock_CommInitReceiveChecksummedData("+$vCont;s:baadfeed;c#", "+$c#");
        mriDebugException(platformMock_GetContext());
    CHECK_TRUE ( Platform_IsSingleStepping() );
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05thread:baadfeed;responseT#+"), platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 0, platformMock_AdvanceProgramCounterToNextInstructionCalls() );
    CHECK_EQUAL( INITIAL_PC, platformMock_GetProgramCounterValue() );

    CHECK_EQUAL( MRI_PLATFORM_THREAD_THAWED, m_threads[0].state );
    CHECK_EQUAL( MRI_PLATFORM_THREAD_THAWED, m_threads[1].state );
    CHECK_EQUAL( MRI_PLATFORM_THREAD_SINGLE_STEPPING, m_threads[2].state );
}

TEST(cmdVCont, vCont_RtosSetThreadStateEnabled_SingleStepHaltedThread)
{
    platformMock_RtosSetHaltedThreadId(0xBAADFEED);
    platformMock_RtosSetIsSetThreadStateSupported(1);
    CHECK_FALSE ( Platform_IsSingleStepping() );
    platformMock_CommInitReceiveChecksummedData("+$vCont;s:baadfeed#", "+$c#");
        mriDebugException(platformMock_GetContext());
    CHECK_TRUE ( Platform_IsSingleStepping() );
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05thread:baadfeed;responseT#+"), platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 0, platformMock_AdvanceProgramCounterToNextInstructionCalls() );
    CHECK_EQUAL( INITIAL_PC, platformMock_GetProgramCounterValue() );

    CHECK_EQUAL( MRI_PLATFORM_THREAD_FROZEN, m_threads[0].state );
    CHECK_EQUAL( MRI_PLATFORM_THREAD_FROZEN, m_threads[1].state );
    CHECK_EQUAL( MRI_PLATFORM_THREAD_SINGLE_STEPPING, m_threads[2].state );
}

TEST(cmdVCont, vCont_RtosSetThreadStateEnabled_SingleStepNotHaltedThreadAndContinue)
{
    platformMock_RtosSetHaltedThreadId(0xBAADFEED);
    platformMock_RtosSetIsSetThreadStateSupported(1);
    CHECK_FALSE ( Platform_IsSingleStepping() );
    platformMock_CommInitReceiveChecksummedData("+$vCont;s:baadf00d;c#", "+$c#");
        mriDebugException(platformMock_GetContext());
    CHECK_TRUE ( Platform_IsSingleStepping() );
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05thread:baadfeed;responseT#+"), platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 0, platformMock_AdvanceProgramCounterToNextInstructionCalls() );
    CHECK_EQUAL( INITIAL_PC, platformMock_GetProgramCounterValue() );

    CHECK_EQUAL( MRI_PLATFORM_THREAD_THAWED, m_threads[0].state );
    CHECK_EQUAL( MRI_PLATFORM_THREAD_SINGLE_STEPPING, m_threads[1].state );
    CHECK_EQUAL( MRI_PLATFORM_THREAD_THAWED, m_threads[2].state );
}

TEST(cmdVCont, vCont_RtosSetThreadStateEnabled_SingleStepNotHaltedThread)
{
    platformMock_RtosSetHaltedThreadId(0xBAADFEED);
    platformMock_RtosSetIsSetThreadStateSupported(1);
    CHECK_FALSE ( Platform_IsSingleStepping() );
    platformMock_CommInitReceiveChecksummedData("+$vCont;s:baadf00d#", "+$c#");
        mriDebugException(platformMock_GetContext());
    CHECK_TRUE ( Platform_IsSingleStepping() );
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05thread:baadfeed;responseT#+"), platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 0, platformMock_AdvanceProgramCounterToNextInstructionCalls() );
    CHECK_EQUAL( INITIAL_PC, platformMock_GetProgramCounterValue() );

    CHECK_EQUAL( MRI_PLATFORM_THREAD_FROZEN, m_threads[0].state );
    CHECK_EQUAL( MRI_PLATFORM_THREAD_SINGLE_STEPPING, m_threads[1].state );
    CHECK_EQUAL( MRI_PLATFORM_THREAD_FROZEN, m_threads[2].state );
}

TEST(cmdVCont, vCont_RtosSetThreadStateEnabled_SingleStepAllThreads)
{
    platformMock_RtosSetHaltedThreadId(0xBAADFEED);
    platformMock_RtosSetIsSetThreadStateSupported(1);
    platformMock_CommInitReceiveChecksummedData("+$vCont;s:-1#", "+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05thread:baadfeed;responseT#+"),
                   platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( MRI_PLATFORM_THREAD_SINGLE_STEPPING, m_threads[0].state );
    CHECK_EQUAL( MRI_PLATFORM_THREAD_SINGLE_STEPPING, m_threads[1].state );
    CHECK_EQUAL( MRI_PLATFORM_THREAD_SINGLE_STEPPING, m_threads[2].state );
}

TEST(cmdVCont, vCont_RtosSetThreadStateEnabled_SingleStepWithNoThreadId)
{
    platformMock_RtosSetHaltedThreadId(0xBAADFEED);
    platformMock_RtosSetIsSetThreadStateSupported(1);
    platformMock_CommInitReceiveChecksummedData("+$vCont;s#", "+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05thread:baadfeed;responseT#+"),
                   platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( MRI_PLATFORM_THREAD_SINGLE_STEPPING, m_threads[0].state );
    CHECK_EQUAL( MRI_PLATFORM_THREAD_SINGLE_STEPPING, m_threads[1].state );
    CHECK_EQUAL( MRI_PLATFORM_THREAD_SINGLE_STEPPING, m_threads[2].state );
}

TEST(cmdVCont, vCont_RtosSetThreadStateEnabled_NoStepOrContinue_ShouldReturnError)
{
    platformMock_RtosSetHaltedThreadId(0xBAADFEED);
    platformMock_RtosSetIsSetThreadStateSupported(1);
    platformMock_CommInitReceiveChecksummedData("+$vCont#", "+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05thread:baadfeed;responseT#+$" MRI_ERROR_INVALID_ARGUMENT "#+"),
                   platformMock_CommGetTransmittedData() );
}

TEST(cmdVCont, vCont_RtosSetThreadStateEnabled_RangedSingleStepHaltedThreadAndContinue)
{
    platformMock_RtosSetHaltedThreadId(0xBAADFEED);
    platformMock_RtosSetIsSetThreadStateSupported(1);
    CHECK_FALSE ( Platform_IsSingleStepping() );
    platformMock_CommInitReceiveChecksummedData("+$vCont;r10000000,10000004:baadfeed;c#");
        mriDebugException(platformMock_GetContext());
    CHECK_TRUE ( Platform_IsSingleStepping() );
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05thread:baadfeed;responseT#+"), platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 0, platformMock_AdvanceProgramCounterToNextInstructionCalls() );
    CHECK_EQUAL( INITIAL_PC, platformMock_GetProgramCounterValue() );
    // Make sure that correct threads were thawed/frozen/single stepped.
    CHECK_EQUAL( MRI_PLATFORM_THREAD_THAWED, m_threads[0].state );
    CHECK_EQUAL( MRI_PLATFORM_THREAD_THAWED, m_threads[1].state );
    CHECK_EQUAL( MRI_PLATFORM_THREAD_SINGLE_STEPPING, m_threads[2].state );
    LONGS_EQUAL ( 0, platformMock_RtosGetRestorePrevThreadStateCallCount() );

    // This stop is still in single step range so should just return after replaying SetThreadState calls.    clearThreadStates();
    Platform_SetProgramCounter(0x10000000);
    platformMock_CommInitTransmitDataBuffer(512);
    platformMock_CommInitReceiveChecksummedData("");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData(""), platformMock_CommGetTransmittedData() );
    LONGS_EQUAL ( 1, platformMock_RtosGetRestorePrevThreadStateCallCount() );

    // This stop is still in single step range so should just return after replaying SetThreadState calls.
    Platform_SetProgramCounter(0x10000002);
    platformMock_CommInitTransmitDataBuffer(512);
    platformMock_CommInitReceiveChecksummedData("");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData(""), platformMock_CommGetTransmittedData() );
    LONGS_EQUAL ( 2, platformMock_RtosGetRestorePrevThreadStateCallCount() );

    // This stop is out of the single step range so it should send T stop response. SetThreadState calls should not be
    // replayed.
    Platform_SetProgramCounter(0x10000004);
    platformMock_CommInitTransmitDataBuffer(512);
    platformMock_CommInitReceiveChecksummedData("+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05thread:baadfeed;responseT#+"), platformMock_CommGetTransmittedData() );
    LONGS_EQUAL ( 2, platformMock_RtosGetRestorePrevThreadStateCallCount() );
}

TEST(cmdVCont, vCont_RtosSetThreadStateEnabled_RangedSingleStepHaltedThreadWithNoContinue)
{
    platformMock_RtosSetHaltedThreadId(0xBAADFEED);
    platformMock_RtosSetIsSetThreadStateSupported(1);
    CHECK_FALSE ( Platform_IsSingleStepping() );
    platformMock_CommInitReceiveChecksummedData("+$vCont;r10000000,10000004:baadfeed#");
        mriDebugException(platformMock_GetContext());
    CHECK_TRUE ( Platform_IsSingleStepping() );
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05thread:baadfeed;responseT#+"), platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 0, platformMock_AdvanceProgramCounterToNextInstructionCalls() );
    CHECK_EQUAL( INITIAL_PC, platformMock_GetProgramCounterValue() );
    // Make sure that correct threads were thawed/frozen/single stepped.
    CHECK_EQUAL( MRI_PLATFORM_THREAD_FROZEN, m_threads[0].state );
    CHECK_EQUAL( MRI_PLATFORM_THREAD_FROZEN, m_threads[1].state );
    CHECK_EQUAL( MRI_PLATFORM_THREAD_SINGLE_STEPPING, m_threads[2].state );
    LONGS_EQUAL ( 0, platformMock_RtosGetRestorePrevThreadStateCallCount() );

    // This stop is still in single step range so should just return after replaying SetThreadState calls.
    Platform_SetProgramCounter(0x10000000);
    platformMock_CommInitTransmitDataBuffer(512);
    platformMock_CommInitReceiveChecksummedData("");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData(""), platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 1, platformMock_RtosGetRestorePrevThreadStateCallCount() );

    // This stop is still in single step range so should just return after replaying SetThreadState calls.
    Platform_SetProgramCounter(0x10000002);
    platformMock_CommInitTransmitDataBuffer(512);
    platformMock_CommInitReceiveChecksummedData("");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData(""), platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 2, platformMock_RtosGetRestorePrevThreadStateCallCount() );

    // This stop is out of the single step range so it should send T stop response. SetThreadState calls should not be
    // replayed.
    Platform_SetProgramCounter(0x10000004);
    platformMock_CommInitTransmitDataBuffer(512);
    platformMock_CommInitReceiveChecksummedData("+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05thread:baadfeed;responseT#+"), platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 2, platformMock_RtosGetRestorePrevThreadStateCallCount() );
}

TEST(cmdVCont, vCont_RtosSetThreadStateEnabled_RangedSingleStepNotHaltedThreadAndContinue)
{
    platformMock_RtosSetHaltedThreadId(0xBAADFEED);
    platformMock_RtosSetIsSetThreadStateSupported(1);
    CHECK_FALSE ( Platform_IsSingleStepping() );
    platformMock_CommInitReceiveChecksummedData("+$vCont;r10000000,10000004:baadf00d;c#");
        mriDebugException(platformMock_GetContext());
    CHECK_TRUE ( Platform_IsSingleStepping() );
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05thread:baadfeed;responseT#+"), platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 0, platformMock_AdvanceProgramCounterToNextInstructionCalls() );
    CHECK_EQUAL( INITIAL_PC, platformMock_GetProgramCounterValue() );
    // Make sure that correct threads were thawed/frozen/single stepped.
    CHECK_EQUAL( MRI_PLATFORM_THREAD_THAWED, m_threads[0].state );
    CHECK_EQUAL( MRI_PLATFORM_THREAD_SINGLE_STEPPING, m_threads[1].state );
    CHECK_EQUAL( MRI_PLATFORM_THREAD_THAWED, m_threads[2].state );
    CHECK_EQUAL( 0, platformMock_RtosGetRestorePrevThreadStateCallCount() );

    // This stop is still in single step range so should just return after replaying SetThreadState calls.
    Platform_SetProgramCounter(0x10000000);
    platformMock_CommInitTransmitDataBuffer(512);
    platformMock_CommInitReceiveChecksummedData("");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData(""), platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 1, platformMock_RtosGetRestorePrevThreadStateCallCount() );

    // This stop is still in single step range so should just return after replaying SetThreadState calls.
    Platform_SetProgramCounter(0x10000002);
    platformMock_CommInitTransmitDataBuffer(512);
    platformMock_CommInitReceiveChecksummedData("");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData(""), platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 2, platformMock_RtosGetRestorePrevThreadStateCallCount() );

    // This stop is out of the single step range so it should send T stop response. SetThreadState calls should not be
    // replayed.
    Platform_SetProgramCounter(0x10000004);
    platformMock_CommInitTransmitDataBuffer(512);
    platformMock_CommInitReceiveChecksummedData("+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05thread:baadfeed;responseT#+"), platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 2, platformMock_RtosGetRestorePrevThreadStateCallCount() );
}

TEST(cmdVCont, vCont_RtosSetThreadStateEnabled_RangedSingleStepNotHaltedThreadWithNoContinue)
{
    platformMock_RtosSetHaltedThreadId(0xBAADFEED);
    platformMock_RtosSetIsSetThreadStateSupported(1);
    CHECK_FALSE ( Platform_IsSingleStepping() );
    platformMock_CommInitReceiveChecksummedData("+$vCont;r10000000,10000004:baadf00d#");
        mriDebugException(platformMock_GetContext());
    CHECK_TRUE ( Platform_IsSingleStepping() );
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05thread:baadfeed;responseT#+"), platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 0, platformMock_AdvanceProgramCounterToNextInstructionCalls() );
    CHECK_EQUAL( INITIAL_PC, platformMock_GetProgramCounterValue() );
    // Make sure that correct threads were thawed/frozen/single stepped.
    CHECK_EQUAL( MRI_PLATFORM_THREAD_FROZEN, m_threads[0].state );
    CHECK_EQUAL( MRI_PLATFORM_THREAD_SINGLE_STEPPING, m_threads[1].state );
    CHECK_EQUAL( MRI_PLATFORM_THREAD_FROZEN, m_threads[2].state );
    CHECK_EQUAL( 0, platformMock_RtosGetRestorePrevThreadStateCallCount() );

    // This stop is still in single step range so should just return after replaying SetThreadState calls.
    Platform_SetProgramCounter(0x10000000);
    platformMock_CommInitTransmitDataBuffer(512);
    platformMock_CommInitReceiveChecksummedData("");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData(""), platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 1, platformMock_RtosGetRestorePrevThreadStateCallCount() );

    // This stop is still in single step range so should just return after replaying SetThreadState calls.
    Platform_SetProgramCounter(0x10000002);
    platformMock_CommInitTransmitDataBuffer(512);
    platformMock_CommInitReceiveChecksummedData("");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData(""), platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 2, platformMock_RtosGetRestorePrevThreadStateCallCount() );

    // This stop is out of the single step range so it should send T stop response. SetThreadState calls should not be
    // replayed.
    Platform_SetProgramCounter(0x10000004);
    platformMock_CommInitTransmitDataBuffer(512);
    platformMock_CommInitReceiveChecksummedData("+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05thread:baadfeed;responseT#+"), platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 2, platformMock_RtosGetRestorePrevThreadStateCallCount() );
}

TEST(cmdVCont, vCont_RtosSetThreadStateEnabled_RangedSingleStepAllThreads_ShouldSingleSteppAllThreads)
{
    platformMock_RtosSetHaltedThreadId(0xBAADFEED);
    platformMock_RtosSetIsSetThreadStateSupported(1);
    platformMock_CommInitReceiveChecksummedData("+$vCont;r10000000,10000004:-1#", "+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05thread:baadfeed;responseT#+"),
                   platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( MRI_PLATFORM_THREAD_SINGLE_STEPPING, m_threads[0].state );
    CHECK_EQUAL( MRI_PLATFORM_THREAD_SINGLE_STEPPING, m_threads[1].state );
    CHECK_EQUAL( MRI_PLATFORM_THREAD_SINGLE_STEPPING, m_threads[2].state );
}

TEST(cmdVCont, vCont_RtosSetThreadStateEnabled_RangedSingleStepWithNoThreadId_ShouldSingleStepAllThreadsByDefault)
{
    platformMock_RtosSetHaltedThreadId(0xBAADFEED);
    platformMock_RtosSetIsSetThreadStateSupported(1);
    platformMock_CommInitReceiveChecksummedData("+$vCont;r10000000,10000004#", "+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05thread:baadfeed;responseT#+"),
                   platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( MRI_PLATFORM_THREAD_SINGLE_STEPPING, m_threads[0].state );
    CHECK_EQUAL( MRI_PLATFORM_THREAD_SINGLE_STEPPING, m_threads[1].state );
    CHECK_EQUAL( MRI_PLATFORM_THREAD_SINGLE_STEPPING, m_threads[2].state );
}

TEST(cmdVCont, vCont_RtosSetThreadStateEnabled_RangedSingleStepHaltedThreadIdWithNoContinue_SkipHardcodedBreakpoint)
{
    platformMock_RtosSetHaltedThreadId(0xBAADFEED);
    platformMock_RtosSetIsSetThreadStateSupported(1);
    platformMock_SetTypeOfCurrentInstruction(MRI_PLATFORM_INSTRUCTION_HARDCODED_BREAKPOINT);
    platformMock_CommInitReceiveChecksummedData("+$vCont;r10000000,10000004:baadfeed#", "+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05thread:baadfeed;responseT#"
                                                 "+$T05thread:baadfeed;responseT#+"), platformMock_CommGetTransmittedData() );
    // Called once for single stepping and another time for the continue used to return from mriDebugException.
    CHECK_EQUAL( 2, platformMock_AdvanceProgramCounterToNextInstructionCalls() );
    // Each instruction advance is hardcoded to 4 bytes in the mock.
    CHECK_EQUAL( 0x10000008, platformMock_GetProgramCounterValue() );
    // If single step over hardcoded breakpoint then Platform_RtosSetThreadState() should be called.
    CHECK_EQUAL( MRI_PLATFORM_THREAD_FROZEN, m_threads[0].state );
    CHECK_EQUAL( MRI_PLATFORM_THREAD_FROZEN, m_threads[1].state );
    CHECK_EQUAL( MRI_PLATFORM_THREAD_FROZEN, m_threads[2].state );
}

TEST(cmdVCont, vCont_RtosSetThreadStateEnabled_RangedSingleStepHaltedThreadIdWithContinue_SkipHardcodedBreakpoint)
{
    platformMock_RtosSetHaltedThreadId(0xBAADFEED);
    platformMock_RtosSetIsSetThreadStateSupported(1);
    platformMock_SetTypeOfCurrentInstruction(MRI_PLATFORM_INSTRUCTION_HARDCODED_BREAKPOINT);
    platformMock_CommInitReceiveChecksummedData("+$vCont;r10000000,10000004:baadfeed;c#", "+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05thread:baadfeed;responseT#"
                                                 "+$T05thread:baadfeed;responseT#+"), platformMock_CommGetTransmittedData() );
    // Called once for single stepping and another time for the continue used to return from mriDebugException.
    CHECK_EQUAL( 2, platformMock_AdvanceProgramCounterToNextInstructionCalls() );
    // Each instruction advance is hardcoded to 4 bytes in the mock.
    CHECK_EQUAL( 0x10000008, platformMock_GetProgramCounterValue() );
    // If single step over hardcoded breakpoint then Platform_RtosSetThreadState() should be called.
    CHECK_EQUAL( MRI_PLATFORM_THREAD_FROZEN, m_threads[0].state );
    CHECK_EQUAL( MRI_PLATFORM_THREAD_FROZEN, m_threads[1].state );
    CHECK_EQUAL( MRI_PLATFORM_THREAD_FROZEN, m_threads[2].state );
}

TEST(cmdVCont, vCont_RtosSetThreadStateEnabled_RangedSingleStepNotHaltedThreadId_NoSkipHardcodedBreakpoint)
{
    platformMock_RtosSetHaltedThreadId(0xBAADFEED);
    platformMock_RtosSetIsSetThreadStateSupported(1);
    platformMock_SetTypeOfCurrentInstruction(MRI_PLATFORM_INSTRUCTION_HARDCODED_BREAKPOINT);
    platformMock_CommInitReceiveChecksummedData("+$vCont;r10000000,10000004:baadf00d#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05thread:baadfeed;responseT#+"), platformMock_CommGetTransmittedData() );
    // Shouldn't look at hardcoded breakpoints when single stepping, non-halted thread.
    CHECK_EQUAL( 0, platformMock_AdvanceProgramCounterToNextInstructionCalls() );
    CHECK_EQUAL( INITIAL_PC, platformMock_GetProgramCounterValue() );

    CHECK_EQUAL( MRI_PLATFORM_THREAD_FROZEN, m_threads[0].state );
    CHECK_EQUAL( MRI_PLATFORM_THREAD_SINGLE_STEPPING, m_threads[1].state );
    CHECK_EQUAL( MRI_PLATFORM_THREAD_FROZEN, m_threads[2].state );
}

TEST(cmdVCont, vCont_RtosSetThreadStateEnabled_RangedSingleStepNotHaltedThreadIdWithContinue_SkipHardcodedBreakpoint)
{
    platformMock_RtosSetHaltedThreadId(0xBAADFEED);
    platformMock_RtosSetIsSetThreadStateSupported(1);
    platformMock_SetTypeOfCurrentInstruction(MRI_PLATFORM_INSTRUCTION_HARDCODED_BREAKPOINT);
    platformMock_CommInitReceiveChecksummedData("+$vCont;r10000000,10000004:baadf00d;c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05thread:baadfeed;responseT#+"), platformMock_CommGetTransmittedData() );
    // Continue will skip the hardcoded breakpoint on the halted thread.
    CHECK_EQUAL( 1, platformMock_AdvanceProgramCounterToNextInstructionCalls() );
    CHECK_EQUAL( INITIAL_PC+4, platformMock_GetProgramCounterValue() );
    // Since not single stepping the halted thread, the Platform_RtosSetThreadState() calls should still happen.
    CHECK_EQUAL( MRI_PLATFORM_THREAD_THAWED, m_threads[0].state );
    CHECK_EQUAL( MRI_PLATFORM_THREAD_SINGLE_STEPPING, m_threads[1].state );
    CHECK_EQUAL( MRI_PLATFORM_THREAD_THAWED, m_threads[2].state );
}

TEST(cmdVCont, vCont_RtosSetThreadStateEnabled_SingleStepHaltedThreadIdWithNoContinue_SkipHardcodedBreakpoint)
{
    platformMock_RtosSetHaltedThreadId(0xBAADFEED);
    platformMock_RtosSetIsSetThreadStateSupported(1);
    platformMock_SetTypeOfCurrentInstruction(MRI_PLATFORM_INSTRUCTION_HARDCODED_BREAKPOINT);
    platformMock_CommInitReceiveChecksummedData("+$vCont;s:baadfeed#", "+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05thread:baadfeed;responseT#"
                                                 "+$T05thread:baadfeed;responseT#+"), platformMock_CommGetTransmittedData() );
    // Called once for single stepping and another time for the continue used to return from mriDebugException.
    CHECK_EQUAL( 2, platformMock_AdvanceProgramCounterToNextInstructionCalls() );
    // Each instruction advance is hardcoded to 4 bytes in the mock.
    CHECK_EQUAL( 0x10000008, platformMock_GetProgramCounterValue() );
    // If single step over hardcoded breakpoint then Platform_RtosSetThreadState() should be called.
    CHECK_EQUAL( MRI_PLATFORM_THREAD_FROZEN, m_threads[0].state );
    CHECK_EQUAL( MRI_PLATFORM_THREAD_FROZEN, m_threads[1].state );
    CHECK_EQUAL( MRI_PLATFORM_THREAD_FROZEN, m_threads[2].state );
}

TEST(cmdVCont, vCont_RtosSetThreadStateEnabled_SingleStepHaltedThreadIdWithContinue_SkipHardcodedBreakpoint)
{
    platformMock_RtosSetHaltedThreadId(0xBAADFEED);
    platformMock_RtosSetIsSetThreadStateSupported(1);
    platformMock_SetTypeOfCurrentInstruction(MRI_PLATFORM_INSTRUCTION_HARDCODED_BREAKPOINT);
    platformMock_CommInitReceiveChecksummedData("+$vCont;s:baadfeed;c#", "+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05thread:baadfeed;responseT#"
                                                 "+$T05thread:baadfeed;responseT#+"), platformMock_CommGetTransmittedData() );
    // Called once for single stepping and another time for the continue used to return from mriDebugException.
    CHECK_EQUAL( 2, platformMock_AdvanceProgramCounterToNextInstructionCalls() );
    // Each instruction advance is hardcoded to 4 bytes in the mock.
    CHECK_EQUAL( 0x10000008, platformMock_GetProgramCounterValue() );
    // If single step over hardcoded breakpoint then Platform_RtosSetThreadState() should be called.
    CHECK_EQUAL( MRI_PLATFORM_THREAD_FROZEN, m_threads[0].state );
    CHECK_EQUAL( MRI_PLATFORM_THREAD_FROZEN, m_threads[1].state );
    CHECK_EQUAL( MRI_PLATFORM_THREAD_FROZEN, m_threads[2].state );
}

TEST(cmdVCont, vCont_RtosSetThreadStateEnabled_SingleStepNotHaltedThreadId_NoSkipHardcodedBreakpoint)
{
    platformMock_RtosSetHaltedThreadId(0xBAADFEED);
    platformMock_RtosSetIsSetThreadStateSupported(1);
    platformMock_SetTypeOfCurrentInstruction(MRI_PLATFORM_INSTRUCTION_HARDCODED_BREAKPOINT);
    platformMock_CommInitReceiveChecksummedData("+$vCont;s:baadf00d#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05thread:baadfeed;responseT#+"), platformMock_CommGetTransmittedData() );
    // Shouldn't look at hardcoded breakpoints when single stepping, non-halted thread.
    CHECK_EQUAL( 0, platformMock_AdvanceProgramCounterToNextInstructionCalls() );
    CHECK_EQUAL( INITIAL_PC, platformMock_GetProgramCounterValue() );

    CHECK_EQUAL( MRI_PLATFORM_THREAD_FROZEN, m_threads[0].state );
    CHECK_EQUAL( MRI_PLATFORM_THREAD_SINGLE_STEPPING, m_threads[1].state );
    CHECK_EQUAL( MRI_PLATFORM_THREAD_FROZEN, m_threads[2].state );
}

TEST(cmdVCont, vCont_RtosSetThreadStateEnabled_SingleStepNotHaltedThreadIdWithContinue_SkipHardcodedBreakpoint)
{
    platformMock_RtosSetHaltedThreadId(0xBAADFEED);
    platformMock_RtosSetIsSetThreadStateSupported(1);
    platformMock_SetTypeOfCurrentInstruction(MRI_PLATFORM_INSTRUCTION_HARDCODED_BREAKPOINT);
    platformMock_CommInitReceiveChecksummedData("+$vCont;s:baadf00d;c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05thread:baadfeed;responseT#+"), platformMock_CommGetTransmittedData() );
    // Continue will skip the hardcoded breakpoint on the halted thread.
    CHECK_EQUAL( 1, platformMock_AdvanceProgramCounterToNextInstructionCalls() );
    CHECK_EQUAL( INITIAL_PC+4, platformMock_GetProgramCounterValue() );
    // Since not single stepping the halted thread, the Platform_RtosSetThreadState() calls should still happen.
    CHECK_EQUAL( MRI_PLATFORM_THREAD_THAWED, m_threads[0].state );
    CHECK_EQUAL( MRI_PLATFORM_THREAD_SINGLE_STEPPING, m_threads[1].state );
    CHECK_EQUAL( MRI_PLATFORM_THREAD_THAWED, m_threads[2].state );
}
