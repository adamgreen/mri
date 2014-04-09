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
#include <string.h>

extern "C"
{
#include "platforms.h"
#include "semihost.h"
#include "try_catch.h"
#include "token.h"
}
#include "platformMock.h"

// Include C++ headers for test harness.
#include "CppUTest/TestHarness.h"

TEST_GROUP(platformMock)
{
    Token   m_token;
    
    void setup()
    {
        platformMock_Init();
        Token_Init(&m_token);
    }

    void teardown()
    {
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

    CHECK_FALSE( Platform_CommHasReceiveData() );
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

    CHECK_FALSE( Platform_CommHasReceiveData() );
}

TEST(platformMock, TransmitAndCapture1Byte)
{
    platformMock_CommInitTransmitDataBuffer(2);

    Platform_CommSendChar('-');

    CHECK_TRUE( platformMock_CommDoesTransmittedDataEqual("-") );
}

TEST(platformMock, TransmitAndCapture2BytesWithOverflow)
{
    platformMock_CommInitTransmitDataBuffer(2);

    Platform_CommSendChar('-');
    Platform_CommSendChar('+');
    Platform_CommSendChar('*');

    CHECK_TRUE( platformMock_CommDoesTransmittedDataEqual("-+") );
}

TEST(platformMock, TransmitAndFailToCompareByLength)
{
    platformMock_CommInitTransmitDataBuffer(2);

    Platform_CommSendChar('-');

    CHECK_FALSE( platformMock_CommDoesTransmittedDataEqual("-+") );
}

TEST(platformMock, TransmitAndFailToCompareByData)
{
    platformMock_CommInitTransmitDataBuffer(2);

    Platform_CommSendChar('-');

    CHECK_FALSE( platformMock_CommDoesTransmittedDataEqual("+") );
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

TEST(platformMock, Platform_CommCausedInterruptReturnsFalseByDefault)
{
    CHECK_FALSE( Platform_CommCausedInterrupt() );
}

TEST(platformMock, Platform_CommCausedInterruptReturnsTrueAfterSettingInMock)
{
    platformMock_CommSetInterruptBit(1);
    CHECK_TRUE( Platform_CommCausedInterrupt() );
}

TEST(platformMock, Platform_CommCausedInterruptReturnsFalseAfterClearing)
{
    platformMock_CommSetInterruptBit(1);
    CHECK_TRUE( Platform_CommCausedInterrupt() );
    Platform_CommClearInterrupt();
    CHECK_FALSE( Platform_CommCausedInterrupt() );
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

TEST(platformMock, Platform_CommShouldWaitForGdbConnect_ReturnsFalseByDefault)
{
    CHECK_FALSE( Platform_CommShouldWaitForGdbConnect() );
}

TEST(platformMock, Platform_CommShouldWaitForGdbConnect_ReturnsTrueAfterSettingInMock)
{
    CHECK_FALSE( Platform_CommShouldWaitForGdbConnect() );
        platformMock_CommSetShouldWaitForGdbConnect(1);
    CHECK_TRUE( Platform_CommShouldWaitForGdbConnect() );
}

TEST(platformMock, Platform_CommIsWaitingForGdbToConnect_ReturnsFalseByDefault)
{
    CHECK_FALSE( Platform_CommIsWaitingForGdbToConnect() );
}

TEST(platformMock, Platform_CommIsWaitingForGdbToConnect_ReturnsTrueDefinedNumberOfTimes)
{
    platformMock_CommSetIsWaitingForGdbToConnectIterations(2);
    CHECK_TRUE( Platform_CommIsWaitingForGdbToConnect() );
    CHECK_TRUE( Platform_CommIsWaitingForGdbToConnect() );
    CHECK_FALSE( Platform_CommIsWaitingForGdbToConnect() );
    CHECK_FALSE( Platform_CommIsWaitingForGdbToConnect() );
}

TEST(platformMock, Platform_CommWaitForReceiveDataToStop_CountCalls)
{
    CHECK_EQUAL( 0, platformMock_GetCommWaitForReceiveDataToStopCalls() );
        Platform_CommWaitForReceiveDataToStop();
    CHECK_EQUAL( 1, platformMock_GetCommWaitForReceiveDataToStopCalls() );
        Platform_CommWaitForReceiveDataToStop();
    CHECK_EQUAL( 2, platformMock_GetCommWaitForReceiveDataToStopCalls() );
}

TEST(platformMock, Platform_CommPrepareToWaitForGdbConnection_CountCalls)
{
    CHECK_EQUAL( 0, platformMock_GetCommPrepareToWaitForGdbConnectionCalls() );
        Platform_CommPrepareToWaitForGdbConnection();
    CHECK_EQUAL( 1, platformMock_GetCommPrepareToWaitForGdbConnectionCalls() );
        Platform_CommPrepareToWaitForGdbConnection();
    CHECK_EQUAL( 2, platformMock_GetCommPrepareToWaitForGdbConnectionCalls() );
}

TEST(platformMock, Semihost_HandleSemihostRequest_CountCalls)
{
    CHECK_EQUAL( 0, platformMock_GetHandleSemihostRequestCalls() );
        CHECK_TRUE( Semihost_HandleSemihostRequest() );
    CHECK_EQUAL( 1, platformMock_GetHandleSemihostRequestCalls() );
        CHECK_TRUE( Semihost_HandleSemihostRequest() );
    CHECK_EQUAL( 2, platformMock_GetHandleSemihostRequestCalls() );
}

TEST(platformMock, Semihost_IsDebuggeeMakingSemihostCall_ReturnsTrueByDefault)
{
    CHECK_TRUE( Semihost_IsDebuggeeMakingSemihostCall() );
}

TEST(platformMock, Semihost_IsDebuggeeMakingSemihostCall_ReturnsFalseAfterSettingMock)
{
    platformMock_SetIsDebuggeeMakingSemihostCall(0);
    CHECK_FALSE( Semihost_IsDebuggeeMakingSemihostCall() );
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
    static const uint32_t expectedSize = 1 + 17 * sizeof(uint32_t) * 2;
    CHECK_EQUAL (expectedSize, Platform_GetPacketBufferSize() );
}

TEST(platformMock, Platform_GetPacketBufferSize_SetSmallSize)
{
    platformMock_SetPacketBufferSize(2);
    CHECK_EQUAL (2, Platform_GetPacketBufferSize() );
}

TEST(platformMock, Platform_CommSharingWithApplication_DefaultsToFalse)
{
    CHECK_FALSE ( Platform_CommSharingWithApplication() );
}

TEST(platformMock, Platform_CommSharingWithApplication_SetToReturnTrue)
{
    platformMock_SetCommSharingWithApplication(1);
    CHECK_TRUE ( Platform_CommSharingWithApplication() );
}
