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
#include <string.h>

extern "C"
{
#include <core/platforms.h>
#include <core/semihost.h>
#include <core/try_catch.h>
#include <core/token.h>
}
#include "platformMock.h"

// Include C++ headers for test harness.
#include "CppUTest/TestHarness.h"

#define UNTOUCHED_STATE ((PlatformThreadState)-1)


TEST_GROUP(platformMock)
{
    enum { THREAD_COUNT = 3 };

    Token              m_token;
    PlatformMockThread m_threads[THREAD_COUNT];
    uint32_t           m_expectedInvalidAttempts;

    void setup()
    {
        platformMock_Init();
        Token_Init(&m_token);
        m_threads[0].threadId = 0x5A5A5A5A;
        m_threads[0].state = UNTOUCHED_STATE;
        m_threads[1].threadId = 0xBAADF00D;
        m_threads[1].state = UNTOUCHED_STATE;
        m_threads[2].threadId = 0xBAADFEED;
        m_threads[2].state = UNTOUCHED_STATE;
        m_expectedInvalidAttempts = 0;
    }

    void teardown()
    {
        LONGS_EQUAL( m_expectedInvalidAttempts, platformMock_RtosGetThreadStateInvalidAttempts() );
        LONGS_EQUAL( noException , getExceptionCode() );
        platformMock_Uninit();
        clearExceptionCode();
    }

    void validateTokenCopy(Token* pTokenCopy)
    {
        POINTERS_EQUAL( m_token.pTokenSeparators, pTokenCopy->pTokenSeparators );
        LONGS_EQUAL( m_token.tokenCount, pTokenCopy->tokenCount );
        STRCMP_EQUAL( m_token.copyOfString, pTokenCopy->copyOfString );

        for (size_t i = 0 ; i < m_token.tokenCount ; i++)
        {
            STRCMP_EQUAL( m_token.tokenPointers[i], pTokenCopy->tokenPointers[i] );
        }
    }
};

TEST(platformMock, Platform_CommHasRecieveData_Empty)
{
    static const char emptyData[] = "";

    platformMock_CommInitReceiveData(emptyData);
    CHECK_FALSE( Platform_CommHasReceiveData() );
}

TEST(platformMock, Platform_CommHasRecieveData_NotEmpty)
{
    static const char testData[] = "$";

    platformMock_CommInitReceiveData(testData);
    CHECK_TRUE( Platform_CommHasReceiveData() );
}

TEST(platformMock, Platform_CommRecieveChar_NotEmpty)
{
    static const char testData[] = "$";

    platformMock_CommInitReceiveData(testData);
    LONGS_EQUAL( '$', Platform_CommReceiveChar() );
}

TEST(platformMock, Platform_CommHasReceiveData_SwitchToEmptyGdbPacket)
{
    static const char emptyData[] = "";
    static const char emptyGdbPacket[] = "$#00";
    char              buffer[16];
    char*             p = buffer;

    platformMock_CommInitReceiveData(emptyData);
    CHECK_FALSE( Platform_CommHasReceiveData() );
    CHECK_TRUE( Platform_CommHasReceiveData() );

    while (Platform_CommHasReceiveData())
    {
        *p++ = (char)Platform_CommReceiveChar();
    }
    LONGS_EQUAL ( strlen(emptyGdbPacket), (p - buffer) );
    CHECK( 0 == memcmp(emptyGdbPacket, buffer, strlen(emptyGdbPacket)) );
}

TEST(platformMock, Platform_CommReceiveChar_SwitchToEmptyGdbPacket)
{
    static const char emptyData[] = "";
    static const char emptyGdbPacket[] = "$#00";
    char              buffer[16];
    char*             p = buffer;

    platformMock_CommInitReceiveData(emptyData);

    do
    {
        *p++ = (char)Platform_CommReceiveChar();
    }
    while (Platform_CommHasReceiveData());

    LONGS_EQUAL ( strlen(emptyGdbPacket), (p - buffer) );
    CHECK( 0 == memcmp(emptyGdbPacket, buffer, strlen(emptyGdbPacket)) );
}

TEST(platformMock, platformMock_CommReceiveEmptyGdbPacket)
{
    static const char emptyGdbPacket[] = "$#00";
    char              buffer[16];
    char*             p = buffer;

    platformMock_CommInitReceiveData(emptyGdbPacket);

    while (Platform_CommHasReceiveData())
    {
        *p++ = (char)Platform_CommReceiveChar();
    }
    LONGS_EQUAL ( strlen(emptyGdbPacket), (p - buffer) );
    CHECK( 0 == memcmp(emptyGdbPacket, buffer, strlen(emptyGdbPacket)) );
}

TEST(platformMock, platformMock_CommReceive_TwoGdbPackets)
{
    static const char packet1[] = "$packet1#00";
    static const char packet2[] = "$packet2#ff";
    char              buffer[16];
    char*             p = buffer;

    platformMock_CommInitReceiveData(packet1, packet2);

    while (Platform_CommHasReceiveData())
    {
        *p++ = (char)Platform_CommReceiveChar();
    }
    LONGS_EQUAL ( strlen(packet1), (p - buffer) );
    CHECK( 0 == memcmp(packet1, buffer, strlen(packet1)) );

    p = buffer;
    while (Platform_CommHasReceiveData())
    {
        *p++ = (char)Platform_CommReceiveChar();
    }
    LONGS_EQUAL ( strlen(packet2), (p - buffer) );
    CHECK( 0 == memcmp(packet2, buffer, strlen(packet2)) );
}

TEST(platformMock, platformMock_CommReceive_TwoGdbPacketsWithCalculatedCRC)
{
    static const char packet1[] = "$packet1#";
    static const char packet2[] = "$packet2#";
    static const char checksummedPacket1[] = "$packet1#a9";
    static const char checksummedPacket2[] = "$packet2#aa";
    char              buffer[16];
    char*             p = buffer;

    platformMock_CommInitReceiveChecksummedData(packet1, packet2);

    while (Platform_CommHasReceiveData())
    {
        *p++ = (char)Platform_CommReceiveChar();
    }
    LONGS_EQUAL ( strlen(checksummedPacket1), (p - buffer) );
    CHECK( 0 == memcmp(checksummedPacket1, buffer, strlen(checksummedPacket1)) );

    p = buffer;
    while (Platform_CommHasReceiveData())
    {
        *p++ = (char)Platform_CommReceiveChar();
    }
    LONGS_EQUAL ( strlen(checksummedPacket2), (p - buffer) );
    CHECK( 0 == memcmp(checksummedPacket2, buffer, strlen(checksummedPacket2)) );
}

TEST(platformMock, platformMock_CommReceive_ThreeGdbPackets)
{
    static const char packet1[] = "$packet1#00";
    static const char packet2[] = "$packet2#ff";
    static const char packet3[] = "$packet3#80";
    char              buffer[16];
    char*             p = buffer;

    platformMock_CommInitReceiveData(packet1, packet2, packet3);

    while (Platform_CommHasReceiveData())
    {
        *p++ = (char)Platform_CommReceiveChar();
    }
    LONGS_EQUAL ( strlen(packet1), (p - buffer) );
    CHECK( 0 == memcmp(packet1, buffer, strlen(packet1)) );

    p = buffer;
    while (Platform_CommHasReceiveData())
    {
        *p++ = (char)Platform_CommReceiveChar();
    }
    LONGS_EQUAL ( strlen(packet2), (p - buffer) );
    CHECK( 0 == memcmp(packet2, buffer, strlen(packet2)) );

    p = buffer;
    while (Platform_CommHasReceiveData())
    {
        *p++ = (char)Platform_CommReceiveChar();
    }
    LONGS_EQUAL ( strlen(packet3), (p - buffer) );
    CHECK( 0 == memcmp(packet3, buffer, strlen(packet3)) );

    CHECK_FALSE( Platform_CommHasReceiveData() );
}

TEST(platformMock, platformMock_CommReceive_ThreeGdbPacketsWithCalculatedCRC)
{
    static const char packet1[] = "$packet1#";
    static const char packet2[] = "$packet2#";
    static const char packet3[] = "$packet3#";
    static const char checksummedPacket1[] = "$packet1#a9";
    static const char checksummedPacket2[] = "$packet2#aa";
    static const char checksummedPacket3[] = "$packet3#ab";
    char              buffer[16];
    char*             p = buffer;

    platformMock_CommInitReceiveChecksummedData(packet1, packet2, packet3);

    while (Platform_CommHasReceiveData())
    {
        *p++ = (char)Platform_CommReceiveChar();
    }
    LONGS_EQUAL ( strlen(checksummedPacket1), (p - buffer) );
    CHECK( 0 == memcmp(checksummedPacket1, buffer, strlen(checksummedPacket1)) );

    p = buffer;
    while (Platform_CommHasReceiveData())
    {
        *p++ = (char)Platform_CommReceiveChar();
    }
    LONGS_EQUAL ( strlen(checksummedPacket2), (p - buffer) );
    CHECK( 0 == memcmp(checksummedPacket2, buffer, strlen(checksummedPacket2)) );

    p = buffer;
    while (Platform_CommHasReceiveData())
    {
        *p++ = (char)Platform_CommReceiveChar();
    }
    LONGS_EQUAL ( strlen(checksummedPacket3), (p - buffer) );
    CHECK( 0 == memcmp(checksummedPacket3, buffer, strlen(checksummedPacket3)) );

    CHECK_FALSE( Platform_CommHasReceiveData() );
}

TEST(platformMock, CommChecksummedData_NoPacketJustNack)
{
    STRCMP_EQUAL( "-", platformMock_CommChecksumData("-") );
}

TEST(platformMock, CommChecksummedData_Ack_EmptyPacket_Ack)
{
    STRCMP_EQUAL( "+$#00+", platformMock_CommChecksumData("+$#+") );
}

TEST(platformMock, CommChecksummedData_Ack_SmallPacket_Nack)
{
    STRCMP_EQUAL( "+$test#c0-", platformMock_CommChecksumData("+$test#-") );
}

TEST(platformMock, CommGetTransmittedData_Transmit1Byte)
{
    platformMock_CommInitTransmitDataBuffer(1);

    Platform_CommSendChar('-');

    STRCMP_EQUAL( "-", platformMock_CommGetTransmittedData() );
}

TEST(platformMock, CommGetTransmittedData_TransmitEmptyPacket)
{
    platformMock_CommInitTransmitDataBuffer(4);

    Platform_CommSendChar('$');
    Platform_CommSendChar('#');
    Platform_CommSendChar('0');
    Platform_CommSendChar('0');

    STRCMP_EQUAL( "$#00", platformMock_CommGetTransmittedData() );
}

TEST(platformMock, Platform_CommHasDataToTransmit_MockAlwaysReturnsFalseAndGetsCounted)
{
    LONGS_EQUAL( 0, platformMock_CommGetHasTransmitCompletedCallCount() );
    CHECK_TRUE( Platform_CommHasTransmitCompleted() );
    LONGS_EQUAL( 1, platformMock_CommGetHasTransmitCompletedCallCount() );
}

TEST(platformMock, platformMockInit_ThrowException)
{
    int exceptionThrown = 0;
    platformMock_SetInitException(timeoutException);

    __try
       Platform_Init(&m_token);
    __catch
        exceptionThrown = 1;

    LONGS_EQUAL( 1, exceptionThrown );
    LONGS_EQUAL( timeoutException, getExceptionCode() );
    clearExceptionCode();
}

TEST(platformMock, platformMockInit_GetCallCount)
{
    LONGS_EQUAL( 0, platformMock_GetInitCount() );
    Platform_Init(&m_token);
    LONGS_EQUAL( 1, platformMock_GetInitCount() );
}

TEST(platformMock, platformMockInit_GetInitTokenCopy)
{
    Token_SplitString(&m_token, "Test Tokens");

    Platform_Init(&m_token);

    Token* pTokenCopy = platformMock_GetInitTokenCopy();
    validateTokenCopy(pTokenCopy);
}

TEST(platformMock, Platform_EnteringDebugger_CountCalls)
{
    CHECK_EQUAL( 0, platformMock_GetEnteringDebuggerCalls() );
        Platform_EnteringDebugger();
    CHECK_EQUAL( 1, platformMock_GetEnteringDebuggerCalls() );
        Platform_EnteringDebugger();
    CHECK_EQUAL( 2, platformMock_GetEnteringDebuggerCalls() );
}

TEST(platformMock, Platform_LeavingDebugger_CountCalls)
{
    CHECK_EQUAL( 0, platformMock_GetLeavingDebuggerCalls() );
        Platform_LeavingDebugger();
    CHECK_EQUAL( 1, platformMock_GetLeavingDebuggerCalls() );
        Platform_LeavingDebugger();
    CHECK_EQUAL( 2, platformMock_GetLeavingDebuggerCalls() );
}

TEST(platformMock, Semihost_HandleSemihostRequest_CountCalls)
{
    CHECK_EQUAL( 0, platformMock_GetHandleSemihostRequestCalls() );
        CHECK_TRUE( Semihost_HandleSemihostRequest() );
    CHECK_EQUAL( 1, platformMock_GetHandleSemihostRequestCalls() );
        CHECK_TRUE( Semihost_HandleSemihostRequest() );
    CHECK_EQUAL( 2, platformMock_GetHandleSemihostRequestCalls() );
}

TEST(platformMock, Semihost_IsDebuggeeMakingSemihostCall_ReturnsFalseByDefault)
{
    CHECK_FALSE( Semihost_IsDebuggeeMakingSemihostCall() );
}

TEST(platformMock, Semihost_IsDebuggeeMakingSemihostCall_ReturnsTrueAfterSettingMock)
{
    platformMock_SetIsDebuggeeMakingSemihostCall(1);
    CHECK_TRUE( Semihost_IsDebuggeeMakingSemihostCall() );
}

TEST(platformMock, Platform_DisplayFaultCauseToGdbConsole_CountCalls)
{
    CHECK_EQUAL( 0, platformMock_DisplayFaultCauseToGdbConsoleCalls() );
        Platform_DisplayFaultCauseToGdbConsole();
    CHECK_EQUAL( 1, platformMock_DisplayFaultCauseToGdbConsoleCalls() );
        Platform_DisplayFaultCauseToGdbConsole();
    CHECK_EQUAL( 2, platformMock_DisplayFaultCauseToGdbConsoleCalls() );
}

TEST(platformMock, Platform_GetPacketBuffer_DefaultNoReturnNull)
{
    CHECK_TRUE ( Platform_GetPacketBuffer() != NULL );
}

TEST(platformMock, Platform_GetPacketBufferSize_DefaultFitsARMContext)
{
    static const uint32_t expectedSize = 1 + 17 * sizeof(uint32_t) * 2 + 4;
    CHECK_EQUAL (expectedSize, Platform_GetPacketBufferSize() );
}

TEST(platformMock, Platform_GetPacketBufferSize_SetSmallSize)
{
    platformMock_SetPacketBufferSize(2);
    CHECK_EQUAL (2, Platform_GetPacketBufferSize() );
}

TEST(platformMock, Platform_TypeOfCurrentInstruction_DefaultsToOther)
{
    CHECK_EQUAL ( MRI_PLATFORM_INSTRUCTION_OTHER, Platform_TypeOfCurrentInstruction() );
}

TEST(platformMock, Platform_TypeOfCurrentInstruction_SetToReturnHardcodedBreakpoint)
{
    platformMock_SetTypeOfCurrentInstruction(MRI_PLATFORM_INSTRUCTION_HARDCODED_BREAKPOINT);
    CHECK_EQUAL ( MRI_PLATFORM_INSTRUCTION_HARDCODED_BREAKPOINT, Platform_TypeOfCurrentInstruction() );
}

TEST(platformMock, Platform_AdvanceProgramCounterToNextInstruction_CountCalls)
{
    CHECK_EQUAL( 0, platformMock_AdvanceProgramCounterToNextInstructionCalls() );
        Platform_AdvanceProgramCounterToNextInstruction();
    CHECK_EQUAL( 1, platformMock_AdvanceProgramCounterToNextInstructionCalls() );
        Platform_AdvanceProgramCounterToNextInstruction();
    CHECK_EQUAL( 2, platformMock_AdvanceProgramCounterToNextInstructionCalls() );
}

TEST(platformMock, Platform_SetProgramCounter_CountCalls)
{
    CHECK_EQUAL( 0, platformMock_SetProgramCounterCalls() );
        Platform_SetProgramCounter(0);
    CHECK_EQUAL( 1, platformMock_SetProgramCounterCalls() );
        Platform_SetProgramCounter(0);
    CHECK_EQUAL( 2, platformMock_SetProgramCounterCalls() );
}

TEST(platformMock, Platform_GetProgramCounterValue_AfterSetAndAdvance)
{
    CHECK_EQUAL( INITIAL_PC, platformMock_GetProgramCounterValue() );
        Platform_SetProgramCounter(0x10008000);
    CHECK_EQUAL( 0x10008000, platformMock_GetProgramCounterValue() );
        Platform_AdvanceProgramCounterToNextInstruction();
    CHECK_EQUAL( 0x10008004, platformMock_GetProgramCounterValue() );
}

TEST(platformMock, Platform_EnableSingleStep)
{
    CHECK_FALSE ( Platform_IsSingleStepping() );
        Platform_EnableSingleStep();
    CHECK_TRUE ( Platform_IsSingleStepping() );
}

TEST(platformMock, Platform_WasMemoryFaultEncountered_DefaultToFalse)
{
    CHECK_FALSE ( Platform_WasMemoryFaultEncountered() );
}

TEST(platformMock, Platform_WasMemoryFaultEncountered_FailFirstCallOnly)
{
    platformMock_FaultOnSpecificMemoryCall(1);
    CHECK_TRUE ( Platform_WasMemoryFaultEncountered() );
    CHECK_FALSE ( Platform_WasMemoryFaultEncountered() );
}

TEST(platformMock, Platform_WasMemoryFaultEncountered_FailSecondCallOnly)
{
    platformMock_FaultOnSpecificMemoryCall(2);
    CHECK_FALSE ( Platform_WasMemoryFaultEncountered() );
    CHECK_TRUE ( Platform_WasMemoryFaultEncountered() );
    CHECK_FALSE ( Platform_WasMemoryFaultEncountered() );
}

TEST(platformMock, Platform_GetContext)
{
    uint32_t* pContextEntries = platformMock_GetContextEntries();
    MriContext* pContext = platformMock_GetContext();

    pContextEntries[0] = 0x11111111;
    pContextEntries[1] = 0x22222222;
    pContextEntries[2] = 0x33333333;
    pContextEntries[3] = 0x44444444;
    LONGS_EQUAL ( 0x11111111, Context_Get(pContext, 0) );
    LONGS_EQUAL ( 0x22222222, Context_Get(pContext, 1) );
    LONGS_EQUAL ( 0x33333333, Context_Get(pContext, 2) );
    LONGS_EQUAL ( 0x44444444, Context_Get(pContext, 3) );

    Context_Set(pContext, 0, 0xAAAAAAAA);
    Context_Set(pContext, 1, 0xBBBBBBBB);
    Context_Set(pContext, 2, 0xCCCCCCCC);
    Context_Set(pContext, 3, 0xDDDDDDDD);
    LONGS_EQUAL ( 0xAAAAAAAA, pContextEntries[0] );
    LONGS_EQUAL ( 0xBBBBBBBB, pContextEntries[1] );
    LONGS_EQUAL ( 0xCCCCCCCC, pContextEntries[2] );
    LONGS_EQUAL ( 0xDDDDDDDD, pContextEntries[3] );
}

TEST(platformMock, Platform_SetHardwareBreakpointOfGdbKind_CountCallsAndLastArgs)
{
    CHECK_EQUAL( 0, platformMock_SetHardwareBreakpointCalls() );
    CHECK_EQUAL( 0, platformMock_SetHardwareBreakpointAddressArg() );
    CHECK_EQUAL( 0, platformMock_SetHardwareBreakpointKindArg() );
        Platform_SetHardwareBreakpointOfGdbKind(0xDEADBEEF, 0xBAADF00D);
    CHECK_EQUAL( 1, platformMock_SetHardwareBreakpointCalls() );
    CHECK_EQUAL( 0xDEADBEEF, platformMock_SetHardwareBreakpointAddressArg() );
    CHECK_EQUAL( 0xBAADF00D, platformMock_SetHardwareBreakpointKindArg() );
        Platform_SetHardwareBreakpointOfGdbKind(0xBAADF00D, 0xDEADBEEF);
    CHECK_EQUAL( 2, platformMock_SetHardwareBreakpointCalls() );
    CHECK_EQUAL( 0xBAADF00D, platformMock_SetHardwareBreakpointAddressArg() );
    CHECK_EQUAL( 0xDEADBEEF, platformMock_SetHardwareBreakpointKindArg() );
}

TEST(platformMock, Platform_SetHardwareBreakpointOfGdbKind_Throwing)
{
    platformMock_SetHardwareBreakpointException(invalidArgumentException);
        Platform_SetHardwareBreakpointOfGdbKind(0xDEADBEEF, 0xBAADF00D);
    CHECK_EQUAL( invalidArgumentException, getExceptionCode() );
    clearExceptionCode();
}

TEST(platformMock, Platform_ClearHardwareBreakpointOfGdbKind_CountCallsAndLastArgs)
{
    CHECK_EQUAL( 0, platformMock_ClearHardwareBreakpointCalls() );
    CHECK_EQUAL( 0, platformMock_ClearHardwareBreakpointAddressArg() );
    CHECK_EQUAL( 0, platformMock_ClearHardwareBreakpointKindArg() );
        Platform_ClearHardwareBreakpointOfGdbKind(0xDEADBEEF, 0xBAADF00D);
    CHECK_EQUAL( 1, platformMock_ClearHardwareBreakpointCalls() );
    CHECK_EQUAL( 0xDEADBEEF, platformMock_ClearHardwareBreakpointAddressArg() );
    CHECK_EQUAL( 0xBAADF00D, platformMock_ClearHardwareBreakpointKindArg() );
        Platform_ClearHardwareBreakpointOfGdbKind(0xBAADF00D, 0xDEADBEEF);
    CHECK_EQUAL( 2, platformMock_ClearHardwareBreakpointCalls() );
    CHECK_EQUAL( 0xBAADF00D, platformMock_ClearHardwareBreakpointAddressArg() );
    CHECK_EQUAL( 0xDEADBEEF, platformMock_ClearHardwareBreakpointKindArg() );
}

TEST(platformMock, Platform_ClearHardwareBreakpointOfGdbKind_Throwing)
{
    platformMock_ClearHardwareBreakpointException(invalidArgumentException);
        Platform_ClearHardwareBreakpointOfGdbKind(0xDEADBEEF, 0xBAADF00D);
    CHECK_EQUAL( invalidArgumentException, getExceptionCode() );
    clearExceptionCode();
}

TEST(platformMock, Platform_SetHardwareWatchpoint_CountCallsAndLastArgs)
{
    CHECK_EQUAL( 0, platformMock_SetHardwareWatchpointCalls() );
    CHECK_EQUAL( 0, platformMock_SetHardwareWatchpointAddressArg() );
    CHECK_EQUAL( 0, platformMock_SetHardwareWatchpointSizeArg() );
    CHECK_EQUAL( MRI_PLATFORM_WRITE_WATCHPOINT, platformMock_SetHardwareWatchpointTypeArg() );
        Platform_SetHardwareWatchpoint(0xDEADBEEF, 0xBAADF00D, MRI_PLATFORM_READ_WATCHPOINT);
    CHECK_EQUAL( 1, platformMock_SetHardwareWatchpointCalls() );
    CHECK_EQUAL( 0xDEADBEEF, platformMock_SetHardwareWatchpointAddressArg() );
    CHECK_EQUAL( 0xBAADF00D, platformMock_SetHardwareWatchpointSizeArg() );
    CHECK_EQUAL( MRI_PLATFORM_READ_WATCHPOINT, platformMock_SetHardwareWatchpointTypeArg() );
        Platform_SetHardwareWatchpoint(0xBAADF00D, 0xDEADBEEF, MRI_PLATFORM_READWRITE_WATCHPOINT);
    CHECK_EQUAL( 2, platformMock_SetHardwareWatchpointCalls() );
    CHECK_EQUAL( 0xBAADF00D, platformMock_SetHardwareWatchpointAddressArg() );
    CHECK_EQUAL( 0xDEADBEEF, platformMock_SetHardwareWatchpointSizeArg() );
    CHECK_EQUAL( MRI_PLATFORM_READWRITE_WATCHPOINT, platformMock_SetHardwareWatchpointTypeArg() );
}

TEST(platformMock, Platform_SetHardwareWatchpoint_Throwing)
{
    platformMock_SetHardwareWatchpointException(invalidArgumentException);
        Platform_SetHardwareWatchpoint(0xDEADBEEF, 0xBAADF00D, MRI_PLATFORM_READWRITE_WATCHPOINT);
    CHECK_EQUAL( invalidArgumentException, getExceptionCode() );
    clearExceptionCode();
}

TEST(platformMock, Platform_ClearHardwareWatchpoint_CountCallsAndLastArgs)
{
    CHECK_EQUAL( 0, platformMock_ClearHardwareWatchpointCalls() );
    CHECK_EQUAL( 0, platformMock_ClearHardwareWatchpointAddressArg() );
    CHECK_EQUAL( 0, platformMock_ClearHardwareWatchpointSizeArg() );
    CHECK_EQUAL( MRI_PLATFORM_WRITE_WATCHPOINT, platformMock_ClearHardwareWatchpointTypeArg() );
        Platform_ClearHardwareWatchpoint(0xDEADBEEF, 0xBAADF00D, MRI_PLATFORM_READ_WATCHPOINT);
    CHECK_EQUAL( 1, platformMock_ClearHardwareWatchpointCalls() );
    CHECK_EQUAL( 0xDEADBEEF, platformMock_ClearHardwareWatchpointAddressArg() );
    CHECK_EQUAL( 0xBAADF00D, platformMock_ClearHardwareWatchpointSizeArg() );
    CHECK_EQUAL( MRI_PLATFORM_READ_WATCHPOINT, platformMock_ClearHardwareWatchpointTypeArg() );
        Platform_ClearHardwareWatchpoint(0xBAADF00D, 0xDEADBEEF, MRI_PLATFORM_READWRITE_WATCHPOINT);
    CHECK_EQUAL( 2, platformMock_ClearHardwareWatchpointCalls() );
    CHECK_EQUAL( 0xBAADF00D, platformMock_ClearHardwareWatchpointAddressArg() );
    CHECK_EQUAL( 0xDEADBEEF, platformMock_ClearHardwareWatchpointSizeArg() );
    CHECK_EQUAL( MRI_PLATFORM_READWRITE_WATCHPOINT, platformMock_ClearHardwareWatchpointTypeArg() );
}

TEST(platformMock, Platform_ClearHardwareWatchpoint_Throwing)
{
    platformMock_ClearHardwareWatchpointException(invalidArgumentException);
        Platform_ClearHardwareWatchpoint(0xDEADBEEF, 0xBAADF00D, MRI_PLATFORM_READWRITE_WATCHPOINT);
    CHECK_EQUAL( invalidArgumentException, getExceptionCode() );
    clearExceptionCode();
}

TEST(platformMock, Platform_GetDeviceMemoryMapXmlSize_Returns4)
{
    CHECK_EQUAL ( 4, Platform_GetDeviceMemoryMapXmlSize() );
}

TEST(platformMock, Platform_GetDeviceMemoryMapXml)
{
    CHECK_EQUAL ( 0, strcmp("TEST", Platform_GetDeviceMemoryMapXml()) );
}

TEST(platformMock, Platform_GetTargetXmlSize_Returns5)
{
    CHECK_EQUAL ( 5, Platform_GetTargetXmlSize() );
}

TEST(platformMock, Platform_GetTargetXml)
{
    CHECK_EQUAL ( 0, strcmp("test!", Platform_GetTargetXml()) );
}

TEST(platformMock, Platform_SetSemihostCallReturnValue_GetValue)
{
    CHECK_EQUAL ( 0, platformMock_GetSemihostCallReturnValue() );
        Platform_SetSemihostCallReturnAndErrnoValues(0x12345678, 0x87654321);
    CHECK_EQUAL ( 0x12345678, platformMock_GetSemihostCallReturnValue() );
    CHECK_EQUAL ( (int)0x87654321, platformMock_GetSemihostCallErrno() );
}


TEST(platformMock, Platform_ResetDevice_CountCalls)
{
    CHECK_EQUAL( 0, platformMock_GetResetDeviceCalls() );
        Platform_ResetDevice();
    CHECK_EQUAL( 1, platformMock_GetResetDeviceCalls() );
}


TEST(platformMock, Platform_DetermineCauseOfException_DefaultsToSIGTRAP)
{
    CHECK_EQUAL( SIGTRAP, Platform_DetermineCauseOfException() );
}

TEST(platformMock, Platform_DetermineCauseOfException_SetToReturnSIGINT)
{
    platformMock_SetCauseOfException(SIGINT);
    CHECK_EQUAL( SIGINT, Platform_DetermineCauseOfException() );
}

TEST(platformMock, Platform_GetTrapResponse_DefaultsToUnknownTrapType)
{
    PlatformTrapReason reason = Platform_GetTrapReason();
    CHECK_EQUAL ( MRI_PLATFORM_TRAP_TYPE_UNKNOWN, reason.type );
}

TEST(platformMock, Platform_GetTrapResponse_MakeSureMockCanReturnOtherTypesAndNonZeroAddresses)
{
    PlatformTrapReason reasonSet;
    PlatformTrapReason reasonActual;

    // HWBREAK
    reasonSet.type = MRI_PLATFORM_TRAP_TYPE_HWBREAK;
    reasonSet.address = 0xFFFFFFFF;
    platformMock_SetTrapReason(&reasonSet);
    reasonActual = Platform_GetTrapReason();
    CHECK_EQUAL( MRI_PLATFORM_TRAP_TYPE_HWBREAK, reasonActual.type );
    CHECK_EQUAL( 0xFFFFFFFF, reasonActual.address );

    // SWBREAK
    reasonSet.type = MRI_PLATFORM_TRAP_TYPE_SWBREAK;
    reasonSet.address = 0x00001000;
    platformMock_SetTrapReason(&reasonSet);
    reasonActual = Platform_GetTrapReason();
    CHECK_EQUAL( MRI_PLATFORM_TRAP_TYPE_SWBREAK, reasonActual.type );
    CHECK_EQUAL( 0x00001000, reasonActual.address );

    // WATCH
    reasonSet.type = MRI_PLATFORM_TRAP_TYPE_WATCH;
    reasonSet.address = 0x20000000;
    platformMock_SetTrapReason(&reasonSet);
    reasonActual = Platform_GetTrapReason();
    CHECK_EQUAL( MRI_PLATFORM_TRAP_TYPE_WATCH, reasonActual.type );
    CHECK_EQUAL( 0x20000000, reasonActual.address );

    // RWATCH
    reasonSet.type = MRI_PLATFORM_TRAP_TYPE_RWATCH;
    reasonSet.address = 0x20000002;
    platformMock_SetTrapReason(&reasonSet);
    reasonActual = Platform_GetTrapReason();
    CHECK_EQUAL( MRI_PLATFORM_TRAP_TYPE_RWATCH, reasonActual.type );
    CHECK_EQUAL( 0x20000002, reasonActual.address );

    // AWATCH
    reasonSet.type = MRI_PLATFORM_TRAP_TYPE_AWATCH;
    reasonSet.address = 0x2FFFFFFC;
    platformMock_SetTrapReason(&reasonSet);
    reasonActual = Platform_GetTrapReason();
    CHECK_EQUAL( MRI_PLATFORM_TRAP_TYPE_AWATCH, reasonActual.type );
    CHECK_EQUAL( 0x2FFFFFFC, reasonActual.address );
}



TEST(platformMock, Platform_RtosGetHaltedThreadId_MockDefaultsToZero)
{
    CHECK_EQUAL( 0, Platform_RtosGetHaltedThreadId() );
}

TEST(platformMock, Platform_RtosGetHaltedThreadId_MakeSureMockCanReturnNonZeroValue)
{
    platformMock_RtosSetHaltedThreadId(0xBAADF00D);
    CHECK_EQUAL( 0xBAADF00D, Platform_RtosGetHaltedThreadId() );
}

TEST(platformMock, Platform_RtosGetFirstThreadId_MockDefaultsToNull)
{
    CHECK_EQUAL( 0, Platform_RtosGetFirstThreadId() )
}

TEST(platformMock, Platform_RtosGetFirstThreadId_ReturnFirstThread)
{
    uint32_t testArray[] = { 0x12345678 };

    platformMock_RtosSetThreads(testArray, sizeof(testArray)/sizeof(testArray[0]));
    CHECK_EQUAL( 0x12345678, Platform_RtosGetFirstThreadId() )
}

TEST(platformMock, Platform_RtosGetNextThreadIdWhenOnlyOneThread_ShouldReturn0)
{
    uint32_t testArray[] = { 0x12345678 };

    platformMock_RtosSetThreads(testArray, sizeof(testArray)/sizeof(testArray[0]));
    CHECK_EQUAL( 0x12345678, Platform_RtosGetFirstThreadId() )
    CHECK_EQUAL( 0, Platform_RtosGetNextThreadId() )
}

TEST(platformMock, Platform_RtosGetNextThreadIdWhenTwoThreads)
{
    uint32_t testArray[] = { 0x12345678, 0xbaadf00d };

    platformMock_RtosSetThreads(testArray, sizeof(testArray)/sizeof(testArray[0]));
    CHECK_EQUAL( 0x12345678, Platform_RtosGetFirstThreadId() )
    CHECK_EQUAL( 0xbaadf00d, Platform_RtosGetNextThreadId() )
    CHECK_EQUAL( 0, Platform_RtosGetNextThreadId() )
}

TEST(platformMock, Platform_RtosGetNextThreadId_SkipNullThread)
{
    uint32_t testArray[] = { 0x12345678, 0x00000000, 0xbaadf00d };

    platformMock_RtosSetThreads(testArray, sizeof(testArray)/sizeof(testArray[0]));
    CHECK_EQUAL( 0x12345678, Platform_RtosGetFirstThreadId() )
    CHECK_EQUAL( 0xbaadf00d, Platform_RtosGetNextThreadId() )
    CHECK_EQUAL( 0, Platform_RtosGetNextThreadId() )
}

TEST(platformMock, Platform_RtosGetExtraThreadInfo_MockDefaultsToNULL)
{
    CHECK_EQUAL( NULL, Platform_RtosGetExtraThreadInfo(0xbaadbeef) );
}

TEST(platformMock, Platform_RtosGetExtraThreadInfo_ReturnsSetStringWhenThreadIdMatches_AndNullWhenItDoesNot)
{
    const char* pTestString = "TestString";

    platformMock_RtosSetExtraThreadInfo(0xbaadbeef, pTestString);
    CHECK_EQUAL( pTestString, Platform_RtosGetExtraThreadInfo(0xbaadbeef) );
    CHECK_EQUAL( NULL, Platform_RtosGetExtraThreadInfo(0xbaadf00d) );
}

TEST(platformMock, Platform_RtosGetThreadContext_MockDefaultsToNULL)
{
    CHECK_EQUAL( NULL, Platform_RtosGetThreadContext(0xbaadbeef) );
}

TEST(platformMock, Platform_RtosGetThreadContext_ReturnsSetContextWhenThreadIdMatches_AndNullWhenItDoesNot)
{
    MriContext context = { NULL, 0 };

    platformMock_RtosSetThreadContext(0xbaadbeef, &context);
    CHECK_EQUAL( &context, Platform_RtosGetThreadContext(0xbaadbeef) );
    CHECK_EQUAL( NULL, Platform_RtosGetThreadContext(0xbaadf00d) );
}

TEST(platformMock, Platform_RtosIsThreadAcive_MockDefaultsToFalse)
{
    CHECK_FALSE( Platform_RtosIsThreadActive(0xbaadbeef) );
}

TEST(platformMock, Platform_RtosIsThreadActive_ReturnsTrueWhenThreadIdMatches_AndFalseWhenItDoesNot)
{
    platformMock_RtosSetActiveThread(0xbaadbeef);
    CHECK_TRUE( Platform_RtosIsThreadActive(0xbaadbeef) );
    CHECK_FALSE( Platform_RtosIsThreadActive(0xbaadf00d) );
}

TEST(platformMock, Platform_RtosIsSetThreadStateSupported_MockDefaultsToFalse)
{
    CHECK_FALSE( Platform_RtosIsSetThreadStateSupported() );
}

TEST(platformMock, Platform_RtosIsSetThreadStateSupported_ReturnsTrueAfterSetting)
{
    platformMock_RtosSetIsSetThreadStateSupported(1);
    CHECK_TRUE( Platform_RtosIsSetThreadStateSupported() );
}

TEST(platformMock, Platform_RtosSetThreadState_SetSpecificThreadToAllStates_VerifyCorrectStateSetInArray)
{
    platformMock_RtosSetThreadList(m_threads, THREAD_COUNT);
    Platform_RtosSetThreadState(0xBAADF00D, MRI_PLATFORM_THREAD_FROZEN);
    LONGS_EQUAL( MRI_PLATFORM_THREAD_FROZEN, m_threads[1].state );
    Platform_RtosSetThreadState(0xBAADF00D, MRI_PLATFORM_THREAD_THAWED);
    LONGS_EQUAL( MRI_PLATFORM_THREAD_THAWED, m_threads[1].state );
    Platform_RtosSetThreadState(0xBAADF00D, MRI_PLATFORM_THREAD_SINGLE_STEPPING);
    LONGS_EQUAL( MRI_PLATFORM_THREAD_SINGLE_STEPPING, m_threads[1].state );

    // Other threads shouldn't be modified.
    LONGS_EQUAL( UNTOUCHED_STATE, m_threads[0].state );
    LONGS_EQUAL( UNTOUCHED_STATE, m_threads[2].state );
}

TEST(platformMock, Platform_RtosSetThreadState_SetAllThreadsToAllStates_VerifyCorrectStateSetInArray)
{
    platformMock_RtosSetThreadList(m_threads, THREAD_COUNT);

    Platform_RtosSetThreadState(MRI_PLATFORM_ALL_THREADS, MRI_PLATFORM_THREAD_THAWED);
    LONGS_EQUAL( MRI_PLATFORM_THREAD_THAWED, m_threads[0].state );
    LONGS_EQUAL( MRI_PLATFORM_THREAD_THAWED, m_threads[1].state );
    LONGS_EQUAL( MRI_PLATFORM_THREAD_THAWED, m_threads[2].state );

    Platform_RtosSetThreadState(MRI_PLATFORM_ALL_THREADS, MRI_PLATFORM_THREAD_FROZEN);
    LONGS_EQUAL( MRI_PLATFORM_THREAD_FROZEN, m_threads[0].state );
    LONGS_EQUAL( MRI_PLATFORM_THREAD_FROZEN, m_threads[1].state );
    LONGS_EQUAL( MRI_PLATFORM_THREAD_FROZEN, m_threads[2].state );

    Platform_RtosSetThreadState(MRI_PLATFORM_ALL_THREADS, MRI_PLATFORM_THREAD_SINGLE_STEPPING);
    LONGS_EQUAL( MRI_PLATFORM_THREAD_SINGLE_STEPPING, m_threads[0].state );
    LONGS_EQUAL( MRI_PLATFORM_THREAD_SINGLE_STEPPING, m_threads[1].state );
    LONGS_EQUAL( MRI_PLATFORM_THREAD_SINGLE_STEPPING, m_threads[2].state );
}

TEST(platformMock, Platform_RtosSetThreadState_SetAllFrozenThreadsToThawed_VerifyCorrectStateSetInArray)
{
    m_threads[0].state = MRI_PLATFORM_THREAD_FROZEN;
    m_threads[1].state = MRI_PLATFORM_THREAD_SINGLE_STEPPING;
    m_threads[2].state = MRI_PLATFORM_THREAD_FROZEN;
    platformMock_RtosSetThreadList(m_threads, THREAD_COUNT);

    Platform_RtosSetThreadState(MRI_PLATFORM_ALL_FROZEN_THREADS, MRI_PLATFORM_THREAD_THAWED);
    LONGS_EQUAL( MRI_PLATFORM_THREAD_THAWED, m_threads[0].state );
    LONGS_EQUAL( MRI_PLATFORM_THREAD_SINGLE_STEPPING, m_threads[1].state );
    LONGS_EQUAL( MRI_PLATFORM_THREAD_THAWED, m_threads[2].state );
}

TEST(platformMock, Platform_RtosSetThreadState_SetOnInvalidThread_VerifyInvalidAttemptCountIs1)
{
    platformMock_RtosSetThreadList(m_threads, THREAD_COUNT);
    Platform_RtosSetThreadState(0x11223344, MRI_PLATFORM_THREAD_FROZEN);
    m_expectedInvalidAttempts = 1;
    LONGS_EQUAL( 1, platformMock_RtosGetThreadStateInvalidAttempts() );

    // None of the threads should be modified.
    LONGS_EQUAL( UNTOUCHED_STATE, m_threads[0].state );
    LONGS_EQUAL( UNTOUCHED_STATE, m_threads[1].state );
    LONGS_EQUAL( UNTOUCHED_STATE, m_threads[2].state );
}


TEST(platformMock, Platform_RtosRestorePrevThreadState_VerifyCallCount)
{
    LONGS_EQUAL( 0, platformMock_RtosGetRestorePrevThreadStateCallCount() );
    Platform_RtosRestorePrevThreadState();
    LONGS_EQUAL( 1, platformMock_RtosGetRestorePrevThreadStateCallCount() );
    Platform_RtosRestorePrevThreadState();
    LONGS_EQUAL( 2, platformMock_RtosGetRestorePrevThreadStateCallCount() );
}
