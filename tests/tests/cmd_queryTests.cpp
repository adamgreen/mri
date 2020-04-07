/* Copyright 2014 Adam Green (https://github.com/adamgreen/)

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

void mriDebugException(void);
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

TEST(cmdQuery, QuerySupported)
{
    platformMock_CommInitReceiveChecksummedData("+$qSupported#", "+$c#");
        mriDebugException();
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#"
                                                 "+$qXfer:memory-map:read+;qXfer:features:read+;PacketSize=89#+"),
                                                 platformMock_CommGetTransmittedData() );
}

TEST(cmdQuery, QueryUnknown_ShouldReturnEmptyResponse)
{
    platformMock_CommInitReceiveChecksummedData("+$qUnknown#", "+$c#");
        mriDebugException();
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$#+"), platformMock_CommGetTransmittedData() );
}

TEST(cmdQuery, QueryUnknownXfer_ShouldReturnEmptyResponse)
{
    platformMock_CommInitReceiveChecksummedData("+$qXfer:unknown#", "+$c#");
        mriDebugException();
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$#+"), platformMock_CommGetTransmittedData() );
}

TEST(cmdQuery, QueryXfer_NoParameters_ShouldReturnErrorResponse)
{
    platformMock_CommInitReceiveChecksummedData("+$qXfer#", "+$c#");
        mriDebugException();
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$" MRI_ERROR_INVALID_ARGUMENT "#+"),
                   platformMock_CommGetTransmittedData() );
}

TEST(cmdQuery, QueryXferMemoryMap_InvalidOperation_ShouldReturnErrorResponse)
{
    platformMock_CommInitReceiveChecksummedData("+$qXfer:memory-map:unknown#", "+$c#");
        mriDebugException();
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$" MRI_ERROR_INVALID_ARGUMENT "#+"),
                   platformMock_CommGetTransmittedData() );
}

TEST(cmdQuery, QueryXferMemoryMap_UnknownAnnex_ShouldReturnErrorResponse)
{
    platformMock_CommInitReceiveChecksummedData("+$qXfer:memory-map:read:unknown:#", "+$c#");
        mriDebugException();
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$" MRI_ERROR_INVALID_ARGUMENT "#+"),
                   platformMock_CommGetTransmittedData() );
}

TEST(cmdQuery, QueryXferMemoryMap_TruncatedAnnex_ShouldReturnErrorResponse)
{
    platformMock_CommInitReceiveChecksummedData("+$qXfer:memory-map:read:target.xml#", "+$c#");
        mriDebugException();
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$" MRI_ERROR_INVALID_ARGUMENT "#+"),
                   platformMock_CommGetTransmittedData() );
}

TEST(cmdQuery, QueryXferMemoryMap_NullAnnexWithAddressButNoLength_ShouldReturnErrorResponse)
{
    platformMock_CommInitReceiveChecksummedData("+$qXfer:memory-map:read::0#", "+$c#");
        mriDebugException();
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$" MRI_ERROR_INVALID_ARGUMENT "#+"),
                   platformMock_CommGetTransmittedData() );
}

TEST(cmdQuery, QueryXferMemoryMap_NonNullAnnex_ShouldReturnErrorResponse)
{
    platformMock_CommInitReceiveChecksummedData("+$qXfer:memory-map:read:target.xml:0,0#", "+$c#");
        mriDebugException();
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$" MRI_ERROR_INVALID_ARGUMENT "#+"),
                   platformMock_CommGetTransmittedData() );
}

TEST(cmdQuery, QueryXferMemoryMap_ReadFirstTwoBytes)
{
    platformMock_CommInitReceiveChecksummedData("+$qXfer:memory-map:read::0,2#", "+$c#");
        mriDebugException();
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$mTE#+"), platformMock_CommGetTransmittedData() );
}

TEST(cmdQuery, QueryXferMemoryMap_ReadLastTwoBytes)
{
    platformMock_CommInitReceiveChecksummedData("+$qXfer:memory-map:read::2,2#", "+$c#");
        mriDebugException();
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$mST#+"), platformMock_CommGetTransmittedData() );
}

TEST(cmdQuery, QueryXferMemoryMap_ReadThroughEnd)
{
    platformMock_CommInitReceiveChecksummedData("+$qXfer:memory-map:read::2,3#", "+$c#");
        mriDebugException();
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$lST#+"), platformMock_CommGetTransmittedData() );
}

TEST(cmdQuery, QueryXferMemoryMap_StartReadPastEnd)
{
    platformMock_CommInitReceiveChecksummedData("+$qXfer:memory-map:read::4,1#", "+$c#");
        mriDebugException();
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$l#+"), platformMock_CommGetTransmittedData() );
}

TEST(cmdQuery, QueryXferMemoryMap_VeryLargeRead)
{
    platformMock_CommInitReceiveChecksummedData("+$qXfer:memory-map:read::0,256#", "+$c#");
        mriDebugException();
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$lTEST#+"), platformMock_CommGetTransmittedData() );
}

TEST(cmdQuery, QueryXferFeatures_InvalidOperation_ShouldReturnErrorResponse)
{
    platformMock_CommInitReceiveChecksummedData("+$qXfer:features:unknown#", "+$c#");
        mriDebugException();
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$" MRI_ERROR_INVALID_ARGUMENT "#+"),
                   platformMock_CommGetTransmittedData() );
}

TEST(cmdQuery, QueryXferFeatures_NullAnnex_ShouldReturnErrorResponse)
{
    platformMock_CommInitReceiveChecksummedData("+$qXfer:features:read::0,0#", "+$c#");
        mriDebugException();
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$" MRI_ERROR_INVALID_ARGUMENT "#+"),
                   platformMock_CommGetTransmittedData() );
}

TEST(cmdQuery, QueryXferFeatures_UnknownAnnex_ShouldReturnErrorResponse)
{
    platformMock_CommInitReceiveChecksummedData("+$qXfer:features:read:unknown:0,0#", "+$c#");
        mriDebugException();
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$" MRI_ERROR_INVALID_ARGUMENT "#+"),
                   platformMock_CommGetTransmittedData() );
}

TEST(cmdQuery, QueryXferFeatures_ReadAllData)
{
    platformMock_CommInitReceiveChecksummedData("+$qXfer:features:read:target.xml:0,5#", "+$c#");
        mriDebugException();
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$mtest!#+"),
                   platformMock_CommGetTransmittedData() );
}

TEST(cmdQuery, QueryXferFeatures_VeryLargeRead)
{
    platformMock_CommInitReceiveChecksummedData("+$qXfer:features:read:target.xml:0,256#", "+$c#");
        mriDebugException();
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$ltest!#+"),
                   platformMock_CommGetTransmittedData() );
}
