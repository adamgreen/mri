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
#include <cmd_file.h>
#include <core.h>
}
#include <platformMock.h>

// Include C++ headers for test harness.
#include "CppUTest/TestHarness.h"


TEST_GROUP(cmdFile)
{
    int  m_expectedException;            
    
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

TEST(cmdFile, IssueGdbFileOpenRequest_ReturnSuccess)
{
    OpenParameters params = { (const char*)(size_t)0x11111111, 0x22222222, 0x33333333, 0x44444444 };
    platformMock_CommInitReceiveChecksummedData("+$F0#");
        IssueGdbFileOpenRequest(&params);
    CHECK_TRUE ( platformMock_CommDoesTransmittedDataEqual("$Fopen,11111111/22222223,33333333,44444444#fc+") );
    CHECK_EQUAL ( 0, platformMock_GetSemihostCallReturnValue() );
    CHECK_FALSE ( WasControlCFlagSentFromGdb() );
    CHECK_FALSE ( WasSemihostCallCancelledByGdb() );
    CHECK_EQUAL ( 0, GetSemihostReturnCode() );
    CHECK_EQUAL ( 0, GetSemihostErrno() );
    CHECK_EQUAL ( 1, platformMock_AdvanceProgramCounterToNextInstructionCalls() );
}

TEST(cmdFile, IssueGdbFileOpenRequest_ReturnError)
{
    OpenParameters params = { (const char*)(size_t)0x11111111, 0x22222222, 0x33333333, 0x44444444 };
    platformMock_CommInitReceiveChecksummedData("+$F-1,12345678#");
        IssueGdbFileOpenRequest(&params);
    CHECK_TRUE ( platformMock_CommDoesTransmittedDataEqual("$Fopen,11111111/22222223,33333333,44444444#fc+") );
    CHECK_EQUAL ( 0xFFFFFFFF, platformMock_GetSemihostCallReturnValue() );
    CHECK_FALSE ( WasControlCFlagSentFromGdb() );
    CHECK_FALSE ( WasSemihostCallCancelledByGdb() );
    CHECK_EQUAL ( -1, GetSemihostReturnCode() );
    CHECK_EQUAL ( 0x12345678, GetSemihostErrno() );
    CHECK_EQUAL ( 1, platformMock_AdvanceProgramCounterToNextInstructionCalls() );
}

TEST(cmdFile, IssueGdbFileOpenRequest_ReturnErrorAndControlC)
{
    OpenParameters params = { (const char*)(size_t)0x11111111, 0x22222222, 0x33333333, 0x44444444 };
    platformMock_CommInitReceiveChecksummedData("+$F-1,12345678,C#");
        IssueGdbFileOpenRequest(&params);
    CHECK_TRUE ( platformMock_CommDoesTransmittedDataEqual("$Fopen,11111111/22222223,33333333,44444444#fc+") );
    CHECK_EQUAL ( 0xFFFFFFFF, platformMock_GetSemihostCallReturnValue() );
    CHECK_TRUE ( WasControlCFlagSentFromGdb() );
    CHECK_FALSE ( WasSemihostCallCancelledByGdb() );
    CHECK_EQUAL ( -1, GetSemihostReturnCode() );
    CHECK_EQUAL ( 0x12345678, GetSemihostErrno() );
    CHECK_EQUAL ( 1, platformMock_AdvanceProgramCounterToNextInstructionCalls() );
}

TEST(cmdFile, IssueGdbFileOpenRequest_ReturnInterruptErrorAndControlC)
{
    OpenParameters params = { (const char*)(size_t)0x11111111, 0x22222222, 0x33333333, 0x44444444 };
    platformMock_CommInitReceiveChecksummedData("+$F-1,4,C#"); // 4 is EINTR
        IssueGdbFileOpenRequest(&params);
    CHECK_TRUE ( platformMock_CommDoesTransmittedDataEqual("$Fopen,11111111/22222223,33333333,44444444#fc+") );
    CHECK_TRUE ( WasControlCFlagSentFromGdb() );
    CHECK_TRUE ( WasSemihostCallCancelledByGdb() );
    CHECK_EQUAL ( -1, GetSemihostReturnCode() );
    CHECK_EQUAL ( 4, GetSemihostErrno() );
    CHECK_EQUAL ( 0, platformMock_GetSemihostCallReturnValue() ); // Doesn't call in this scenario.
    CHECK_EQUAL ( 0, platformMock_AdvanceProgramCounterToNextInstructionCalls() );
}

TEST(cmdFile, IssueGdbFileWriteRequest_ReturnSuccess)
{
    TransferParameters params = { 0x11111111, 0x22222222, 0x33333333 };
    platformMock_CommInitReceiveChecksummedData("+$F0#");
        IssueGdbFileWriteRequest(&params);
    CHECK_TRUE ( platformMock_CommDoesTransmittedDataEqual("$Fwrite,11111111,22222222,33333333#a5+") );
    CHECK_EQUAL ( 0, platformMock_GetSemihostCallReturnValue() );
    CHECK_FALSE ( WasControlCFlagSentFromGdb() );
    CHECK_FALSE ( WasSemihostCallCancelledByGdb() );
    CHECK_EQUAL ( 0, GetSemihostReturnCode() );
    CHECK_EQUAL ( 0, GetSemihostErrno() );
    CHECK_EQUAL ( 1, platformMock_AdvanceProgramCounterToNextInstructionCalls() );
}

TEST(cmdFile, IssueGdbFileReadRequest_ReturnSuccess)
{
    TransferParameters params = { 0x11111111, 0x22222222, 0x33333333 };
    platformMock_CommInitReceiveChecksummedData("+$F0#");
        IssueGdbFileReadRequest(&params);
    CHECK_TRUE ( platformMock_CommDoesTransmittedDataEqual("$Fread,11111111,22222222,33333333#16+") );
    CHECK_EQUAL ( 0, platformMock_GetSemihostCallReturnValue() );
    CHECK_FALSE ( WasControlCFlagSentFromGdb() );
    CHECK_FALSE ( WasSemihostCallCancelledByGdb() );
    CHECK_EQUAL ( 0, GetSemihostReturnCode() );
    CHECK_EQUAL ( 0, GetSemihostErrno() );
    CHECK_EQUAL ( 1, platformMock_AdvanceProgramCounterToNextInstructionCalls() );
}

TEST(cmdFile, IssueGdbFileCloseRequest_ReturnSuccess)
{
    platformMock_CommInitReceiveChecksummedData("+$F0#");
        IssueGdbFileCloseRequest(0x12345678);
    CHECK_TRUE ( platformMock_CommDoesTransmittedDataEqual("$Fclose,12345678#2c+") );
    CHECK_EQUAL ( 0, platformMock_GetSemihostCallReturnValue() );
    CHECK_FALSE ( WasControlCFlagSentFromGdb() );
    CHECK_FALSE ( WasSemihostCallCancelledByGdb() );
    CHECK_EQUAL ( 0, GetSemihostReturnCode() );
    CHECK_EQUAL ( 0, GetSemihostErrno() );
    CHECK_EQUAL ( 1, platformMock_AdvanceProgramCounterToNextInstructionCalls() );
}

TEST(cmdFile, IssueGdbFileSeekRequest_ReturnSuccess)
{
    SeekParameters params = { 0x11111111, 0x22222222, 0x33333333 };
    platformMock_CommInitReceiveChecksummedData("+$F0#");
        IssueGdbFileSeekRequest(&params);
    CHECK_TRUE ( platformMock_CommDoesTransmittedDataEqual("$Flseek,11111111,22222222,33333333#8e+") );
    CHECK_EQUAL ( 0, platformMock_GetSemihostCallReturnValue() );
    CHECK_FALSE ( WasControlCFlagSentFromGdb() );
    CHECK_FALSE ( WasSemihostCallCancelledByGdb() );
    CHECK_EQUAL ( 0, GetSemihostReturnCode() );
    CHECK_EQUAL ( 0, GetSemihostErrno() );
    CHECK_EQUAL ( 1, platformMock_AdvanceProgramCounterToNextInstructionCalls() );
}

TEST(cmdFile, IssueGdbFileFStatRequest_ReturnSuccess)
{
    platformMock_CommInitReceiveChecksummedData("+$F0#");
        IssueGdbFileFStatRequest(0x11111111, 0x22222222);
    CHECK_TRUE ( platformMock_CommDoesTransmittedDataEqual("$Ffstat,11111111,22222222#d8+") );
    CHECK_EQUAL ( 0, platformMock_GetSemihostCallReturnValue() );
    CHECK_FALSE ( WasControlCFlagSentFromGdb() );
    CHECK_FALSE ( WasSemihostCallCancelledByGdb() );
    CHECK_EQUAL ( 0, GetSemihostReturnCode() );
    CHECK_EQUAL ( 0, GetSemihostErrno() );
    CHECK_EQUAL ( 1, platformMock_AdvanceProgramCounterToNextInstructionCalls() );
}

TEST(cmdFile, IssueGdbFileUnlinkRequest_ReturnSuccess)
{
    RemoveParameters params = { 0x11111111, 0x22222222 };
    platformMock_CommInitReceiveChecksummedData("+$F0#");
        IssueGdbFileUnlinkRequest(&params);
    CHECK_TRUE ( platformMock_CommDoesTransmittedDataEqual("$Funlink,11111111/22222223#4b+") );
    CHECK_EQUAL ( 0, platformMock_GetSemihostCallReturnValue() );
    CHECK_FALSE ( WasControlCFlagSentFromGdb() );
    CHECK_FALSE ( WasSemihostCallCancelledByGdb() );
    CHECK_EQUAL ( 0, GetSemihostReturnCode() );
    CHECK_EQUAL ( 0, GetSemihostErrno() );
    CHECK_EQUAL ( 1, platformMock_AdvanceProgramCounterToNextInstructionCalls() );
}

TEST(cmdFile, IssueGdbFileStatRequest_ReturnSuccess)
{
    char filename[] = "filename";
    platformMock_CommInitReceiveChecksummedData("+$F0#");
        IssueGdbFileStatRequest(filename, 0x12345678);
    CHECK_EQUAL ( 0, platformMock_GetSemihostCallReturnValue() );
    CHECK_FALSE ( WasControlCFlagSentFromGdb() );
    CHECK_FALSE ( WasSemihostCallCancelledByGdb() );
    CHECK_EQUAL ( 0, GetSemihostReturnCode() );
    CHECK_EQUAL ( 0, GetSemihostErrno() );
    CHECK_EQUAL ( 1, platformMock_AdvanceProgramCounterToNextInstructionCalls() );
}

TEST(cmdFile, IssueGdbFileRenameRequest_ReturnSuccess)
{
    char origFilename[] = "origFilename";
    char newFilename[] = "newFilename";
    platformMock_CommInitReceiveChecksummedData("+$F0#");
        IssueGdbFileRenameRequest(origFilename, newFilename);
    CHECK_EQUAL ( 0, platformMock_GetSemihostCallReturnValue() );
    CHECK_FALSE ( WasControlCFlagSentFromGdb() );
    CHECK_FALSE ( WasSemihostCallCancelledByGdb() );
    CHECK_EQUAL ( 0, GetSemihostReturnCode() );
    CHECK_EQUAL ( 0, GetSemihostErrno() );
    CHECK_EQUAL ( 1, platformMock_AdvanceProgramCounterToNextInstructionCalls() );
}
