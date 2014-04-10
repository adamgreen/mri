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
#include <platforms.h>

void __mriDebugException(void);
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

TEST(cmdStep, BasicSingleStep)
{
    CHECK_FALSE ( Platform_IsSingleStepping() );
    platformMock_CommInitReceiveChecksummedData("+$s#");
        __mriDebugException();
    CHECK_TRUE ( Platform_IsSingleStepping() );
    CHECK_TRUE ( platformMock_CommDoesTransmittedDataEqual("$T05responseT#7c+") );
    CHECK_EQUAL( 0, platformMock_AdvanceProgramCounterToNextInstructionCalls() );
}

TEST(cmdStep, SingleStepOverHardcodedBreakpoints_MustContinueAfterToExit)
{
    platformMock_SetTypeOfCurrentInstruction(MRI_PLATFORM_INSTRUCTION_HARDCODED_BREAKPOINT);
    platformMock_CommInitReceiveChecksummedData("+$s#", "+$c#");
        __mriDebugException();
    CHECK_TRUE ( platformMock_CommDoesTransmittedDataEqual("$T05responseT#7c+$T05responseT#7c+") );
    CHECK_EQUAL( 2, platformMock_AdvanceProgramCounterToNextInstructionCalls() );
    CHECK_EQUAL( INITIAL_PC + 8, platformMock_GetProgramCounterValue() );
}
