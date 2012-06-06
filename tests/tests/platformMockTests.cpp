/* Copyright 2012 Adam Green (http://mbed.org/users/AdamGreen/)

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
        clearExceptionCode();
        Token_Init(&m_token);
    }

    void teardown()
    {
        LONGS_EQUAL( noException , getExceptionCode() );
        platformMock_CommUninitTransmitDataBuffer();
        platformMock_SetInitException(noException);
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
    platformMock_ClearInitCount();
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

TEST(platformMock, platformMock_GetDisableSingleStepCount)
{
    platformMock_ClearDisableSingleStepCount();
    LONGS_EQUAL( 0, platformMock_GetDisableSingleStepCount() );
    Platform_DisableSingleStep();
    LONGS_EQUAL( 1, platformMock_GetDisableSingleStepCount() );
}
