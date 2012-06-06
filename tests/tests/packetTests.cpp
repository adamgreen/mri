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
#include "packet.h"
#include "try_catch.h"
#include "token.h"
}
#include "platformMock.h"

// Include C++ headers for test harness.
#include "CppUTest/TestHarness.h"

TEST_GROUP(Packet)
{
    Packet            m_packet;
    Buffer            m_buffer;
    char*             m_pCharacterArray;
    int               m_exceptionThrown;
    static const char m_fillChar = 0xFF;
    
    void setup()
    {
        m_pCharacterArray = NULL;
        allocateBuffer(32);
        m_exceptionThrown = 0;
        Packet_Init(&m_packet);
        platformMock_CommInitTransmitDataBuffer(16);
    }

    void teardown()
    {
        validateNoExceptionThrown();
        platformMock_CommUninitTransmitDataBuffer();
        free(m_pCharacterArray);
    }
    
    void validateNoExceptionThrown()
    {
        CHECK_FALSE ( m_exceptionThrown );
        LONGS_EQUAL ( 0, getExceptionCode() );
    }
    
    void allocateBuffer(size_t sizeOfBuffer)
    {
        free(m_pCharacterArray);
        m_pCharacterArray = (char*)malloc(sizeOfBuffer);
        memset(m_pCharacterArray, m_fillChar, sizeOfBuffer);
        Buffer_Init(&m_buffer, m_pCharacterArray, sizeOfBuffer);
    }
    
    void allocateBuffer(const char* pBufferString)
    {
        free(m_pCharacterArray);
        m_pCharacterArray = NULL;
        Buffer_Init(&m_buffer, (char*)pBufferString, strlen(pBufferString));
    }

    void tryPacketGet()
    {
        __try
            Packet_GetFromGDB(&m_packet, &m_buffer);
        __catch
            m_exceptionThrown = 1;
    }
    
    void tryPacketSend()
    {
        __try
            Packet_SendToGDB(&m_packet, &m_buffer);
        __catch
            m_exceptionThrown = 1;
    }
    
    void validateThatEmptyGdbPacketWasRead()
    {
        LONGS_EQUAL( 0, Buffer_GetLength(&m_buffer) );
    }
    
    void validateBufferMatches(const char* pExpectedOutput)
    {
        CHECK_TRUE( Buffer_MatchesString(&m_buffer, pExpectedOutput, strlen(pExpectedOutput)) );
    }
};

TEST(Packet, PacketGetFromGDB_Empty)
{
    platformMock_CommInitReceiveData("$#00");
    tryPacketGet();
    validateThatEmptyGdbPacketWasRead();
    CHECK_TRUE( platformMock_CommDoesTransmittedDataEqual("+") );
}

TEST(Packet, PacketGetFromGDB_Short)
{
    platformMock_CommInitReceiveData("$?#3f");
    tryPacketGet();
    validateBufferMatches("?");
    CHECK_TRUE( platformMock_CommDoesTransmittedDataEqual("+") );
}

TEST(Packet, PacketGetFromGDB_BadChecksum)
{
    platformMock_CommInitReceiveData("$?#f3");
    tryPacketGet();
    validateThatEmptyGdbPacketWasRead();
    CHECK_TRUE( platformMock_CommDoesTransmittedDataEqual("-+") ); 
}

TEST(Packet, PacketGetFromGDB_TwoPackets)
{
    platformMock_CommInitReceiveData("$#00$?#3f");
    tryPacketGet();
    validateBufferMatches("?");
    CHECK_TRUE( platformMock_CommDoesTransmittedDataEqual("+") );
}

TEST(Packet, PacketGetFromGDB_SearchForStartOfPacket)
{
    platformMock_CommInitReceiveData("#00$?#3f");
    tryPacketGet();
    validateBufferMatches("?");
    CHECK_TRUE( platformMock_CommDoesTransmittedDataEqual("+") );
}

TEST(Packet, PacketGetFromGDB_StartOfPacketWithinPacket)
{
    platformMock_CommInitReceiveData("$?$?#3f");
    tryPacketGet();
    validateBufferMatches("?");
    CHECK_TRUE( platformMock_CommDoesTransmittedDataEqual("+") );
}

TEST(Packet, PacketGetFromGDB_BufferTooSmall)
{
    platformMock_CommInitReceiveData("$qSupported:qRelocInsn+#9af");
    allocateBuffer(8);
    tryPacketGet();
    validateThatEmptyGdbPacketWasRead();
    CHECK_TRUE( platformMock_CommDoesTransmittedDataEqual("+") );
}

TEST(Packet, PacketSendToGDB_EmptyWithAck)
{
    allocateBuffer("");
    platformMock_CommInitReceiveData("+");
    tryPacketSend();
    CHECK_TRUE( platformMock_CommDoesTransmittedDataEqual("$#00") );
}

TEST(Packet, PacketSendToGDB_OkWithAck)
{
    allocateBuffer("OK");
    platformMock_CommInitReceiveData("+");
    tryPacketSend();
    CHECK_TRUE( platformMock_CommDoesTransmittedDataEqual("$OK#9a") );
}

TEST(Packet, PacketSendToGDB_OkWithNackAndAck)
{
    allocateBuffer("OK");
    platformMock_CommInitReceiveData("-+");
    tryPacketSend();
    CHECK_TRUE( platformMock_CommDoesTransmittedDataEqual("$OK#9a$OK#9a") );
}

TEST(Packet, PacketSendToGDB_OkWithCancelForNewPacket)
{
    allocateBuffer("OK");
    platformMock_CommInitReceiveData("$");
    tryPacketSend();
    CHECK_TRUE( platformMock_CommDoesTransmittedDataEqual("$OK#9a") );
}
   
