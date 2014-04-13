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

void __mriDebugException(void);
}
#include <platformMock.h>

// Include C++ headers for test harness.
#include "CppUTest/TestHarness.h"


TEST_GROUP(cmdQuery)
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

TEST(cmdQuery, QuerySupported)
{
    platformMock_CommInitReceiveChecksummedData("+$qSupported#", "+$c#");
        __mriDebugException();
    CHECK_TRUE ( platformMock_CommDoesTransmittedDataEqual("$T05responseT#7c"
                                                           "+$qXfer:memory-map:read+;qXfer:features:read+;PacketSize=89#fc+") );
}

TEST(cmdQuery, QueryUnknown_ShouldReturnEmptyResponse)
{
    platformMock_CommInitReceiveChecksummedData("+$qUnknown#", "+$c#");
        __mriDebugException();
    CHECK_TRUE ( platformMock_CommDoesTransmittedDataEqual("$T05responseT#7c+$#00+") );
}

TEST(cmdQuery, QueryUnknownXfer_ShouldReturnEmptyResponse)
{
    platformMock_CommInitReceiveChecksummedData("+$qXfer:unknown#", "+$c#");
        __mriDebugException();
    CHECK_TRUE ( platformMock_CommDoesTransmittedDataEqual("$T05responseT#7c+$#00+") );
}

TEST(cmdQuery, QueryXfer_NoParameters_ShouldReturnErrorResponse)
{
    platformMock_CommInitReceiveChecksummedData("+$qXfer#", "+$c#");
        __mriDebugException();
    CHECK_TRUE ( platformMock_CommDoesTransmittedDataEqual("$T05responseT#7c+$" MRI_ERROR_INVALID_ARGUMENT "#a6+") );
}

TEST(cmdQuery, QueryXferMemoryMap_InvalidOperation_ShouldReturnErrorResponse)
{
    platformMock_CommInitReceiveChecksummedData("+$qXfer:memory-map:unknown#", "+$c#");
        __mriDebugException();
    CHECK_TRUE ( platformMock_CommDoesTransmittedDataEqual("$T05responseT#7c+$" MRI_ERROR_INVALID_ARGUMENT "#a6+") );
}

TEST(cmdQuery, QueryXferMemoryMap_UnknownAnnex_ShouldReturnErrorResponse)
{
    platformMock_CommInitReceiveChecksummedData("+$qXfer:memory-map:read:unknown:#", "+$c#");
        __mriDebugException();
    CHECK_TRUE ( platformMock_CommDoesTransmittedDataEqual("$T05responseT#7c+$" MRI_ERROR_INVALID_ARGUMENT "#a6+") );
}

TEST(cmdQuery, QueryXferMemoryMap_TruncatedAnnex_ShouldReturnErrorResponse)
{
    platformMock_CommInitReceiveChecksummedData("+$qXfer:memory-map:read:target.xml#", "+$c#");
        __mriDebugException();
    CHECK_TRUE ( platformMock_CommDoesTransmittedDataEqual("$T05responseT#7c+$" MRI_ERROR_INVALID_ARGUMENT "#a6+") );
}

TEST(cmdQuery, QueryXferMemoryMap_NullAnnexWithAddressButNoLength_ShouldReturnErrorResponse)
{
    platformMock_CommInitReceiveChecksummedData("+$qXfer:memory-map:read::0#", "+$c#");
        __mriDebugException();
    CHECK_TRUE ( platformMock_CommDoesTransmittedDataEqual("$T05responseT#7c+$" MRI_ERROR_INVALID_ARGUMENT "#a6+") );
}

TEST(cmdQuery, QueryXferMemoryMap_NonNullAnnex_ShouldReturnErrorResponse)
{
    platformMock_CommInitReceiveChecksummedData("+$qXfer:memory-map:read:target.xml:0,0#", "+$c#");
        __mriDebugException();
    CHECK_TRUE ( platformMock_CommDoesTransmittedDataEqual("$T05responseT#7c+$" MRI_ERROR_INVALID_ARGUMENT "#a6+") );
}

TEST(cmdQuery, QueryXferMemoryMap_ReadFirstTwoBytes)
{
    platformMock_CommInitReceiveChecksummedData("+$qXfer:memory-map:read::0,2#", "+$c#");
        __mriDebugException();
    CHECK_TRUE ( platformMock_CommDoesTransmittedDataEqual("$T05responseT#7c+$mTE#06+") );
}

TEST(cmdQuery, QueryXferMemoryMap_ReadLastTwoBytes)
{
    platformMock_CommInitReceiveChecksummedData("+$qXfer:memory-map:read::2,2#", "+$c#");
        __mriDebugException();
    CHECK_TRUE ( platformMock_CommDoesTransmittedDataEqual("$T05responseT#7c+$mST#14+") );
}

TEST(cmdQuery, QueryXferMemoryMap_ReadThroughEnd)
{
    platformMock_CommInitReceiveChecksummedData("+$qXfer:memory-map:read::2,3#", "+$c#");
        __mriDebugException();
    CHECK_TRUE ( platformMock_CommDoesTransmittedDataEqual("$T05responseT#7c+$lST#13+") );
}

TEST(cmdQuery, QueryXferMemoryMap_StartReadPastEnd)
{
    platformMock_CommInitReceiveChecksummedData("+$qXfer:memory-map:read::4,1#", "+$c#");
        __mriDebugException();
    CHECK_TRUE ( platformMock_CommDoesTransmittedDataEqual("$T05responseT#7c+$l#6c+") );
}

TEST(cmdQuery, QueryXferMemoryMap_VeryLargeRead)
{
    platformMock_CommInitReceiveChecksummedData("+$qXfer:memory-map:read::0,256#", "+$c#");
        __mriDebugException();
    CHECK_TRUE ( platformMock_CommDoesTransmittedDataEqual("$T05responseT#7c+$lTEST#ac+") );
}

TEST(cmdQuery, QueryXferFeatures_InvalidOperation_ShouldReturnErrorResponse)
{
    platformMock_CommInitReceiveChecksummedData("+$qXfer:features:unknown#", "+$c#");
        __mriDebugException();
    CHECK_TRUE ( platformMock_CommDoesTransmittedDataEqual("$T05responseT#7c+$" MRI_ERROR_INVALID_ARGUMENT "#a6+") );
}

TEST(cmdQuery, QueryXferFeatures_NullAnnex_ShouldReturnErrorResponse)
{
    platformMock_CommInitReceiveChecksummedData("+$qXfer:features:read::0,0#", "+$c#");
        __mriDebugException();
    CHECK_TRUE ( platformMock_CommDoesTransmittedDataEqual("$T05responseT#7c+$" MRI_ERROR_INVALID_ARGUMENT "#a6+") );
}

TEST(cmdQuery, QueryXferFeatures_UnknownAnnex_ShouldReturnErrorResponse)
{
    platformMock_CommInitReceiveChecksummedData("+$qXfer:features:read:unknown:0,0#", "+$c#");
        __mriDebugException();
    CHECK_TRUE ( platformMock_CommDoesTransmittedDataEqual("$T05responseT#7c+$" MRI_ERROR_INVALID_ARGUMENT "#a6+") );
}

TEST(cmdQuery, QueryXferFeatures_ReadAllData)
{
    platformMock_CommInitReceiveChecksummedData("+$qXfer:features:read:target.xml:0,5#", "+$c#");
        __mriDebugException();
    CHECK_TRUE ( platformMock_CommDoesTransmittedDataEqual("$T05responseT#7c+$mtest!#4e+") );
}

TEST(cmdQuery, QueryXferFeatures_VeryLargeRead)
{
    platformMock_CommInitReceiveChecksummedData("+$qXfer:features:read:target.xml:0,256#", "+$c#");
        __mriDebugException();
    CHECK_TRUE ( platformMock_CommDoesTransmittedDataEqual("$T05responseT#7c+$ltest!#4d+") );
}
