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
#include <core/platforms.h>

void mriDebugException(void);
}
#include <platformMock.h>

// Include C++ headers for test harness.
#include "CppUTest/TestHarness.h"


TEST_GROUP(cmdVCont)
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


TEST(cmdVCont, UnknownCommandStartingWithV_ShouldReturnEmptyResponse)
{
    platformMock_CommInitReceiveChecksummedData("+$vFoo#", "+$c#");
        mriDebugException();
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$#+"), platformMock_CommGetTransmittedData() );
}



// vCont? Tests
TEST(cmdVCont, vContQst_VerifyOutput)
{
    platformMock_CommInitReceiveChecksummedData("+$vCont?#", "+$c#");
        mriDebugException();
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$vCont;c;C;s;S;r#+"), platformMock_CommGetTransmittedData() );
}



// vCont Tests
TEST(cmdVCont, vCont_MissingSemicolonBeforeAction_ShouldReturnError)
{
    platformMock_CommInitReceiveChecksummedData("+$vCont:c#", "+$c#");
        mriDebugException();
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$" MRI_ERROR_INVALID_ARGUMENT "#+"),
                   platformMock_CommGetTransmittedData() );
}

TEST(cmdVCont, vCont_MissingActions_ShouldReturnError)
{
    platformMock_CommInitReceiveChecksummedData("+$vCont#", "+$c#");
        mriDebugException();
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$" MRI_ERROR_INVALID_ARGUMENT "#+"),
                   platformMock_CommGetTransmittedData() );
}

TEST(cmdVCont, vCont_UnknownAction_ShouldReturnError)
{
    platformMock_CommInitReceiveChecksummedData("+$vCont;X#", "+$c#");
        mriDebugException();
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$" MRI_ERROR_INVALID_ARGUMENT "#+"),
                   platformMock_CommGetTransmittedData() );
}

TEST(cmdVCont, vCont_ContinueDefaultAction_SkipOverHardcodedBreakpoint)
{
    platformMock_SetTypeOfCurrentInstruction(MRI_PLATFORM_INSTRUCTION_HARDCODED_BREAKPOINT);
    platformMock_CommInitReceiveChecksummedData("+$vCont;c#");
        mriDebugException();
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+"), platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 1, platformMock_AdvanceProgramCounterToNextInstructionCalls() );
    CHECK_EQUAL( INITIAL_PC + 4, platformMock_GetProgramCounterValue() );
}

TEST(cmdVCont, vCont_ContinueAllThreads_SkipOverHardcodedBreakpoint)
{
    platformMock_SetTypeOfCurrentInstruction(MRI_PLATFORM_INSTRUCTION_HARDCODED_BREAKPOINT);
    platformMock_CommInitReceiveChecksummedData("+$vCont;c:-1#");
        mriDebugException();
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+"), platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 1, platformMock_AdvanceProgramCounterToNextInstructionCalls() );
    CHECK_EQUAL( INITIAL_PC + 4, platformMock_GetProgramCounterValue() );
}

TEST(cmdVCont, vCont_ContinueWithSignalDefaultAction_SkipOverHardcodedBreakpoint)
{
    platformMock_SetTypeOfCurrentInstruction(MRI_PLATFORM_INSTRUCTION_HARDCODED_BREAKPOINT);
    platformMock_CommInitReceiveChecksummedData("+$vCont;Cab#");
        mriDebugException();
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+"), platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 1, platformMock_AdvanceProgramCounterToNextInstructionCalls() );
    CHECK_EQUAL( INITIAL_PC + 4, platformMock_GetProgramCounterValue() );
}

TEST(cmdVCont, vCont_ContinueWithSignalAllThreads_SkipOverHardcodedBreakpoint)
{
    platformMock_SetTypeOfCurrentInstruction(MRI_PLATFORM_INSTRUCTION_HARDCODED_BREAKPOINT);
    platformMock_CommInitReceiveChecksummedData("+$vCont;Cab:-1#");
        mriDebugException();
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+"), platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 1, platformMock_AdvanceProgramCounterToNextInstructionCalls() );
    CHECK_EQUAL( INITIAL_PC + 4, platformMock_GetProgramCounterValue() );
}

TEST(cmdVCont, vCont_ContinueWithMissingSignalValue_ShouldReturnError)
{
    platformMock_SetTypeOfCurrentInstruction(MRI_PLATFORM_INSTRUCTION_HARDCODED_BREAKPOINT);
    platformMock_CommInitReceiveChecksummedData("+$vCont;C#", "+$c#");
        mriDebugException();
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$" MRI_ERROR_INVALID_ARGUMENT "#+"),
                   platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 1, platformMock_AdvanceProgramCounterToNextInstructionCalls() );
    CHECK_EQUAL( INITIAL_PC + 4, platformMock_GetProgramCounterValue() );
}

TEST(cmdVCont, vCont_SingleStepActionOnly)
{
    CHECK_FALSE ( Platform_IsSingleStepping() );
    platformMock_CommInitReceiveChecksummedData("+$vCont;s#");
        mriDebugException();
    CHECK_TRUE ( Platform_IsSingleStepping() );
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+"), platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 0, platformMock_AdvanceProgramCounterToNextInstructionCalls() );
    CHECK_EQUAL( INITIAL_PC, platformMock_GetProgramCounterValue() );
}

TEST(cmdVCont, vCont_SingleStepAllThreads)
{
    CHECK_FALSE ( Platform_IsSingleStepping() );
    platformMock_CommInitReceiveChecksummedData("+$vCont;s:-1#");
        mriDebugException();
    CHECK_TRUE ( Platform_IsSingleStepping() );
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+"), platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 0, platformMock_AdvanceProgramCounterToNextInstructionCalls() );
    CHECK_EQUAL( INITIAL_PC, platformMock_GetProgramCounterValue() );
}

TEST(cmdVCont, vCont_SingleStepAllThreadsAndDefaultOfContinue)
{
    CHECK_FALSE ( Platform_IsSingleStepping() );
    platformMock_CommInitReceiveChecksummedData("+$vCont;s:-1;c#");
        mriDebugException();
    CHECK_TRUE ( Platform_IsSingleStepping() );
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+"), platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 0, platformMock_AdvanceProgramCounterToNextInstructionCalls() );
    CHECK_EQUAL( INITIAL_PC, platformMock_GetProgramCounterValue() );
}

TEST(cmdVCont, vCont_SingleStepWithSignalValueActionOnly)
{
    CHECK_FALSE ( Platform_IsSingleStepping() );
    platformMock_CommInitReceiveChecksummedData("+$vCont;Sab#");
        mriDebugException();
    CHECK_TRUE ( Platform_IsSingleStepping() );
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+"), platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 0, platformMock_AdvanceProgramCounterToNextInstructionCalls() );
    CHECK_EQUAL( INITIAL_PC, platformMock_GetProgramCounterValue() );
}

TEST(cmdVCont, vCont_SingleStepWithSignalValueAllThreads)
{
    CHECK_FALSE ( Platform_IsSingleStepping() );
    platformMock_CommInitReceiveChecksummedData("+$vCont;Sab:-1#");
        mriDebugException();
    CHECK_TRUE ( Platform_IsSingleStepping() );
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+"), platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 0, platformMock_AdvanceProgramCounterToNextInstructionCalls() );
    CHECK_EQUAL( INITIAL_PC, platformMock_GetProgramCounterValue() );
}

TEST(cmdVCont, vCont_SingleStepWithMissingSignalValue_ShouldReturnError)
{
    platformMock_CommInitReceiveChecksummedData("+$vCont;S#", "+$c#");
        mriDebugException();
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$" MRI_ERROR_INVALID_ARGUMENT "#+"),
                   platformMock_CommGetTransmittedData() );
}

TEST(cmdVCont, vCont_RangedSingleStepActionOnly)
{
    CHECK_FALSE ( Platform_IsSingleStepping() );
    platformMock_CommInitReceiveChecksummedData("+$vCont;r10000000,10000004#");
        mriDebugException();
    CHECK_TRUE ( Platform_IsSingleStepping() );
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+"), platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 0, platformMock_AdvanceProgramCounterToNextInstructionCalls() );
    CHECK_EQUAL( INITIAL_PC, platformMock_GetProgramCounterValue() );
}

TEST(cmdVCont, vCont_RangedSingleStepAllThreadsAndContinueAsDefault)
{
    CHECK_FALSE ( Platform_IsSingleStepping() );
    platformMock_CommInitReceiveChecksummedData("+$vCont;r10000000,10000004:-1;c#");
        mriDebugException();
    CHECK_TRUE ( Platform_IsSingleStepping() );
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+"), platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 0, platformMock_AdvanceProgramCounterToNextInstructionCalls() );
    CHECK_EQUAL( INITIAL_PC, platformMock_GetProgramCounterValue() );
}

TEST(cmdVCont, vCont_RangedSingleStep_VerifyOnlySingleStepsInDesiredRange)
{
    CHECK_FALSE ( Platform_IsSingleStepping() );
    platformMock_CommInitReceiveChecksummedData("+$vCont;r10000000,10000004#");
        mriDebugException();
    CHECK_TRUE ( Platform_IsSingleStepping() );
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+"), platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 0, platformMock_AdvanceProgramCounterToNextInstructionCalls() );
    CHECK_EQUAL( INITIAL_PC, platformMock_GetProgramCounterValue() );

    // This stop is still in single step range so should just return.
    Platform_SetProgramCounter(0x10000000);
    platformMock_CommInitTransmitDataBuffer(512);
    platformMock_CommInitReceiveChecksummedData("");
        mriDebugException();
    STRCMP_EQUAL ( platformMock_CommChecksumData(""), platformMock_CommGetTransmittedData() );

    // This stop is still in single step range so should just return.
    Platform_SetProgramCounter(0x10000002);
    platformMock_CommInitTransmitDataBuffer(512);
    platformMock_CommInitReceiveChecksummedData("");
        mriDebugException();
    STRCMP_EQUAL ( platformMock_CommChecksumData(""), platformMock_CommGetTransmittedData() );

    // This stop is out of the single step range so it should send T stop response.
    Platform_SetProgramCounter(0x10000004);
    platformMock_CommInitTransmitDataBuffer(512);
    platformMock_CommInitReceiveChecksummedData("+$c#");
        mriDebugException();
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+"), platformMock_CommGetTransmittedData() );
}

TEST(cmdVCont, vCont_RangedSingleStep_VerifyCanBeInterruptedEvenWhenInRange)
{
    CHECK_FALSE ( Platform_IsSingleStepping() );
    platformMock_CommInitReceiveChecksummedData("+$vCont;r10000000,10000004#");
        mriDebugException();
    CHECK_TRUE ( Platform_IsSingleStepping() );
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+"), platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 0, platformMock_AdvanceProgramCounterToNextInstructionCalls() );
    CHECK_EQUAL( INITIAL_PC, platformMock_GetProgramCounterValue() );

    // This stop is still in single step range so should just return.
    Platform_SetProgramCounter(0x10000000);
    platformMock_CommInitTransmitDataBuffer(512);
    platformMock_CommInitReceiveChecksummedData("");
        mriDebugException();
    STRCMP_EQUAL ( platformMock_CommChecksumData(""), platformMock_CommGetTransmittedData() );

    // Simulate a CTRL+C interrupt as cause and should get T response instead.
    Platform_SetProgramCounter(0x10000002);
    platformMock_SetCauseOfException(SIGINT);
    platformMock_CommInitTransmitDataBuffer(512);
    platformMock_CommInitReceiveChecksummedData("+$c#");
        mriDebugException();
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T02responseT#+"), platformMock_CommGetTransmittedData() );
}

TEST(cmdVCont, vCont_RangedSingleStep_VerifyWillStopForBreakpointpointEvenWhenInRange)
{
    CHECK_FALSE ( Platform_IsSingleStepping() );
    platformMock_CommInitReceiveChecksummedData("+$vCont;r10000000,10000004#");
        mriDebugException();
    CHECK_TRUE ( Platform_IsSingleStepping() );
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+"), platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 0, platformMock_AdvanceProgramCounterToNextInstructionCalls() );
    CHECK_EQUAL( INITIAL_PC, platformMock_GetProgramCounterValue() );

    // This stop is still in single step range so should just return.
    Platform_SetProgramCounter(0x10000000);
    platformMock_CommInitTransmitDataBuffer(512);
    platformMock_CommInitReceiveChecksummedData("");
        mriDebugException();
    STRCMP_EQUAL ( platformMock_CommChecksumData(""), platformMock_CommGetTransmittedData() );

    // Simulate a hardware breakpoint as cause and should get T response instead.
    PlatformTrapReason breakpointReason = { MRI_PLATFORM_TRAP_TYPE_HWBREAK, 0x10000002 };
    Platform_SetProgramCounter(0x10000002);
    platformMock_SetTrapReason(&breakpointReason);
    platformMock_CommInitTransmitDataBuffer(512);
    platformMock_CommInitReceiveChecksummedData("+$c#");
        mriDebugException();
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+"), platformMock_CommGetTransmittedData() );
}

TEST(cmdVCont, vCont_RangedSingleStep_StepOverOneHardcodedBreakpoint_NeedToContinueOverAnotherBreakpointToExit)
{
    platformMock_SetTypeOfCurrentInstruction(MRI_PLATFORM_INSTRUCTION_HARDCODED_BREAKPOINT);
    platformMock_CommInitReceiveChecksummedData("+$vCont;r10000000,10000004#", "+$c#");
        mriDebugException();
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$T05responseT#+"), platformMock_CommGetTransmittedData() );
    // Called once for single stepping and another time for the continue used to return from mriDebugException.
    CHECK_EQUAL( 2, platformMock_AdvanceProgramCounterToNextInstructionCalls() );
    // Each instruction advance is hardcoded to 4 bytes in the mock.
    CHECK_EQUAL( 0x10000008, platformMock_GetProgramCounterValue() );
}

TEST(cmdVCont, vCont_RangedSingleStep_StepOverTwoHardcodedBreakpoints_NeedToContinueOverAnotherBreakpointToExit)
{
    platformMock_SetTypeOfCurrentInstruction(MRI_PLATFORM_INSTRUCTION_HARDCODED_BREAKPOINT);
    platformMock_CommInitReceiveChecksummedData("+$vCont;r10000000,10000008#", "+$c#");
        mriDebugException();
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$T05responseT#+"), platformMock_CommGetTransmittedData() );
    // Called twice for single stepping and another time for the continue used to return from mriDebugException.
    CHECK_EQUAL( 3, platformMock_AdvanceProgramCounterToNextInstructionCalls() );
    // Each instruction advance is hardcoded to 4 bytes in the mock.
    CHECK_EQUAL( 0x1000000C, platformMock_GetProgramCounterValue() );
}

TEST(cmdVCont, vCont_RangedSingleStepWithMissingRange_ShouldReturnError)
{
    platformMock_CommInitReceiveChecksummedData("+$vCont;r#", "+$c#");
        mriDebugException();
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$" MRI_ERROR_INVALID_ARGUMENT "#+"),
                   platformMock_CommGetTransmittedData() );
}

TEST(cmdVCont, vCont_RangedSingleStepWithMissingComma_ShouldReturnError)
{
    platformMock_CommInitReceiveChecksummedData("+$vCont;r10000000#", "+$c#");
        mriDebugException();
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$" MRI_ERROR_INVALID_ARGUMENT "#+"),
                   platformMock_CommGetTransmittedData() );
}

TEST(cmdVCont, vCont_RangedSingleStepWithMissingEndValue_ShouldReturnError)
{
    platformMock_CommInitReceiveChecksummedData("+$vCont;r10000000,#", "+$c#");
        mriDebugException();
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$" MRI_ERROR_INVALID_ARGUMENT "#+"),
                   platformMock_CommGetTransmittedData() );
}

// UNDONE: Testing that specific thread-ids are treated as errors for now.
TEST(cmdVCont, vCont_SingleStepWithSpecificThreadId_ShouldReturnError)
{
    platformMock_CommInitReceiveChecksummedData("+$vCont;s:deadbeef#", "+$c#");
        mriDebugException();
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$" MRI_ERROR_INVALID_ARGUMENT "#+"),
                   platformMock_CommGetTransmittedData() );
}
