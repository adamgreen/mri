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

void mriDebugException(void);
}
#include <platformMock.h>

// Include C++ headers for test harness.
#include "CppUTest/TestHarness.h"


TEST_GROUP(cmdQuery)
{
    int     m_expectedException;
    char*   m_pCommandString;

    void setup()
    {
        m_expectedException = noException;
        m_pCommandString = NULL;
        platformMock_Init();
        mriInit("MRI_UART_MBED_USB");
    }

    void teardown()
    {
        LONGS_EQUAL ( m_expectedException, getExceptionCode() );
        free(m_pCommandString);
        m_pCommandString = NULL;
        clearExceptionCode();
        platformMock_Uninit();
    }

    void validateExceptionCode(int expectedExceptionCode)
    {
        m_expectedException = expectedExceptionCode;
        LONGS_EQUAL ( expectedExceptionCode, getExceptionCode() );
    }

    const char* monitorCommand(const char* pCommand)
    {
        const char commandPrefix[] = "+$qRcmd,";
        size_t len = sizeof(commandPrefix)-1 + 2 * strlen(pCommand) + 1 + 1;
        m_pCommandString = (char*)realloc(m_pCommandString, len);
        memcpy(m_pCommandString, commandPrefix, sizeof(commandPrefix) - 1);
        char* pDest = m_pCommandString + sizeof(commandPrefix) - 1;
        pDest += stringToHex(pDest, pCommand);
        *pDest++ = '#';
        *pDest++ = '\0';
        assert ( pDest - m_pCommandString == (int)len );

        return m_pCommandString;
    }

    int stringToHex(char* pHexDest, const char* pSrc)
    {
        char* pStart = pHexDest;
        while (*pSrc)
        {
            sprintf(pHexDest, "%02x", *pSrc++);
            pHexDest += 2;
        }
        int len = pHexDest - pStart;
        *pHexDest++ = '\0';
        return len;
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





TEST(cmdQuery, QueryRcmd_WithMissingComma_ShouldFail)
{
    platformMock_CommInitReceiveChecksummedData("+$qRcmd#", "+$c#");
        mriDebugException();
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#" "+$" MRI_ERROR_INVALID_ARGUMENT "#+"),
                   platformMock_CommGetTransmittedData() );
}

TEST(cmdQuery, QueryRcmd_Reset_ShouldOutputMessage_MakesSureAckTransmitCompleted_CallsPlatformResetDevice)
{
    const char* pCommand = monitorCommand("reset");
    platformMock_CommInitReceiveChecksummedData(pCommand, "++$c#");
    LONGS_EQUAL( 0, platformMock_GetResetDeviceCalls() );
        mriDebugException();
    char expectedConsoleOutput[256];
    char expectedTransmitData[512];
    stringToHex(expectedConsoleOutput, "Will reset on next continue.\r\n");
    snprintf(expectedTransmitData, sizeof(expectedTransmitData), "$T05responseT#+$O%s#$OK#+", expectedConsoleOutput);
    STRCMP_EQUAL ( platformMock_CommChecksumData(expectedTransmitData),
                   platformMock_CommGetTransmittedData() );
    LONGS_EQUAL( 1, platformMock_GetResetDeviceCalls() );
    LONGS_EQUAL( 1, platformMock_CommGetHasTransmitCompletedCallCount() );
}

TEST(cmdQuery, QueryRcmd_ShowFault_ShouldDumpException)
{
    const char* pCommand = monitorCommand("showfault");
    platformMock_CommInitReceiveChecksummedData(pCommand, "+$c#");
    LONGS_EQUAL( 0, platformMock_DisplayFaultCauseToGdbConsoleCalls() );
        mriDebugException();
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$OK#+"), platformMock_CommGetTransmittedData() );
    LONGS_EQUAL( 1, platformMock_DisplayFaultCauseToGdbConsoleCalls() );
}

TEST(cmdQuery, QueryRcmd_Help_ShouldDisplaySupportedCommands)
{
    const char* pCommand = monitorCommand("help");
    platformMock_CommInitReceiveChecksummedData(pCommand, "++++$c#");
    LONGS_EQUAL( 0, platformMock_GetResetDeviceCalls() );
        mriDebugException();
    char expectedConsoleOutput[3][64];
    char expectedTransmitData[512];
    stringToHex(expectedConsoleOutput[0], "Supported monitor commands:\r\n");
    stringToHex(expectedConsoleOutput[1], "reset\r\n");
    stringToHex(expectedConsoleOutput[2], "showfault\r\n");
    snprintf(expectedTransmitData, sizeof(expectedTransmitData),
             "$T05responseT#+$O%s#$O%s#$O%s#$OK#+",
             expectedConsoleOutput[0],
             expectedConsoleOutput[1],
             expectedConsoleOutput[2]);
    STRCMP_EQUAL ( platformMock_CommChecksumData(expectedTransmitData),
                   platformMock_CommGetTransmittedData() );
}

TEST(cmdQuery, QueryRcmd_UnknownMonitorCommand_ShouldDisplayErrorAndHelp)
{
    const char* pCommand = monitorCommand("unknown");
    platformMock_CommInitReceiveChecksummedData(pCommand, "+++++$c#");
    LONGS_EQUAL( 0, platformMock_GetResetDeviceCalls() );
        mriDebugException();
    char expectedConsoleOutput[4][64];
    char expectedTransmitData[512];
    stringToHex(expectedConsoleOutput[0], "Unrecognized monitor command!\r\n");
    stringToHex(expectedConsoleOutput[1], "Supported monitor commands:\r\n");
    stringToHex(expectedConsoleOutput[2], "reset\r\n");
    stringToHex(expectedConsoleOutput[3], "showfault\r\n");
    snprintf(expectedTransmitData, sizeof(expectedTransmitData),
             "$T05responseT#+$O%s#$O%s#$O%s#$O%s#$OK#+",
             expectedConsoleOutput[0],
             expectedConsoleOutput[1],
             expectedConsoleOutput[2],
             expectedConsoleOutput[3]);
    STRCMP_EQUAL ( platformMock_CommChecksumData(expectedTransmitData),
                   platformMock_CommGetTransmittedData() );
}




TEST(cmdQuery, qfThreadInfo_ShouldReturnEmptyResponseIfZeroThreadCount)
{
    platformMock_RtosSetThreadCount(0);
    platformMock_RtosSetThreadArrayPointer(NULL);
    platformMock_CommInitReceiveChecksummedData("+$qfThreadInfo#", "+$c#");
        mriDebugException();
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#" "+$#+"),
                   platformMock_CommGetTransmittedData() );
}

TEST(cmdQuery, qfThreadInfo_ReturnOneThread)
{
    uint32_t threadIds[] = { 0xBAADBEEF };
    platformMock_RtosSetThreadCount(sizeof(threadIds)/sizeof(threadIds[0]));
    platformMock_RtosSetThreadArrayPointer(threadIds);
    platformMock_CommInitReceiveChecksummedData("+$qfThreadInfo#", "+$c#");
        mriDebugException();
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#" "+$mbaadbeef#+"),
                   platformMock_CommGetTransmittedData() );
}

TEST(cmdQuery, qfThreadInfo_ReturnTwoThreads_WithPacketBufferJustLargeEnoughForBothThreadIds)
{
    uint32_t threadIds[] = { 0x11111111, 0x22222222 };
    platformMock_RtosSetThreadCount(sizeof(threadIds)/sizeof(threadIds[0]));
    platformMock_RtosSetThreadArrayPointer(threadIds);
    platformMock_SetPacketBufferSize(18);
    platformMock_CommInitReceiveChecksummedData("+$qfThreadInfo#", "+$c#");
        mriDebugException();
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#" "+$m11111111,22222222#+"),
                   platformMock_CommGetTransmittedData() );
}

TEST(cmdQuery, qfThreadInfo_UseBufferTooSmallForTwoThreadIds_ShouldTruncateToOneThread)
{
    uint32_t threadIds[] = { 0x11111111, 0x22222222 };
    platformMock_RtosSetThreadCount(sizeof(threadIds)/sizeof(threadIds[0]));
    platformMock_RtosSetThreadArrayPointer(threadIds);
    platformMock_SetPacketBufferSize(17);
    platformMock_CommInitReceiveChecksummedData("+$qfThreadInfo#", "+$c#");
        mriDebugException();
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#" "+$m11111111#+"),
                   platformMock_CommGetTransmittedData() );
}

TEST(cmdQuery, qfThreadInfo_SetThreeThreads_ReturnTwoNonZeroThreads)
{
    uint32_t threadIds[] = { 0x11111111, 0x00000000, 0x22222222 };
    platformMock_RtosSetThreadCount(sizeof(threadIds)/sizeof(threadIds[0]));
    platformMock_RtosSetThreadArrayPointer(threadIds);
    platformMock_CommInitReceiveChecksummedData("+$qfThreadInfo#", "+$c#");
        mriDebugException();
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#" "+$m11111111,22222222#+"),
                   platformMock_CommGetTransmittedData() );
}

TEST(cmdQuery, qsThreadInfo_AfterReturningOneThread_ShouldReturnLtoIndicateLastThreadHasBeenSent)
{
    uint32_t threadIds[] = { 0xBAADBEEF };
    platformMock_RtosSetThreadCount(sizeof(threadIds)/sizeof(threadIds[0]));
    platformMock_RtosSetThreadArrayPointer(threadIds);
    platformMock_CommInitReceiveChecksummedData("+$qfThreadInfo#", "+$qsThreadInfo#", "+$c#");
        mriDebugException();
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#" "+$mbaadbeef#" "+$l#" "+"),
                   platformMock_CommGetTransmittedData() );
}

TEST(cmdQuery, qsThreadInfo_AfterAlreadyReturningFirstThreadOfTwo_ShouldReturnSecondThread)
{
    uint32_t threadIds[] = { 0x11111111, 0x22222222 };
    platformMock_RtosSetThreadCount(sizeof(threadIds)/sizeof(threadIds[0]));
    platformMock_RtosSetThreadArrayPointer(threadIds);
    platformMock_SetPacketBufferSize(12);
    platformMock_CommInitReceiveChecksummedData("+$qfThreadInfo#", "+$qsThreadInfo#", "+$c#");
        mriDebugException();
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#" "+$m11111111#" "+$m22222222#" "+"),
                   platformMock_CommGetTransmittedData() );
}
