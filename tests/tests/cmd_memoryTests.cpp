/* Copyright 2023 Adam Green (https://github.com/adamgreen/)

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
#include <core/core.h>
}
#include <platformMock.h>
#include <stdio.h>

// Include C++ headers for test harness.
#include "CppUTest/TestHarness.h"


TEST_GROUP(cmdMemory)
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

TEST(cmdMemory, MemoryRead64Aligned)
{
    uint64_t value = 0x123456789ABCDEF0;
    char     packet[64];
    snprintf(packet, sizeof(packet), "+$m%016lx,8#", (size_t)&value);
    platformMock_CommInitReceiveChecksummedData(packet, "+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$f0debc9a78563412#+"), platformMock_CommGetTransmittedData() );
}

TEST(cmdMemory, MemoryRead32Aligned)
{
    uint32_t value = 0x12345678;
    char     packet[64];
    snprintf(packet, sizeof(packet), "+$m%016lx,4#", (size_t)&value);
    platformMock_CommInitReceiveChecksummedData(packet, "+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$78563412#+"), platformMock_CommGetTransmittedData() );
}

TEST(cmdMemory, MemoryRead16Aligned)
{
    uint16_t value = 0x1234;
    char     packet[64];
    snprintf(packet, sizeof(packet), "+$m%016lx,2#", (size_t)&value);
    platformMock_CommInitReceiveChecksummedData(packet, "+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$3412#+"), platformMock_CommGetTransmittedData() );
}

TEST(cmdMemory, MemoryRead8)
{
    uint8_t  value = 0x12;
    char     packet[64];
    snprintf(packet, sizeof(packet), "+$m%016lx,1#", (size_t)&value);
    platformMock_CommInitReceiveChecksummedData(packet, "+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$12#+"), platformMock_CommGetTransmittedData() );
}

TEST(cmdMemory, MemoryRead_InvalidParameterSeparator_ErrorResponse)
{
    uint8_t  value = 0x12;
    char     packet[64];
    snprintf(packet, sizeof(packet), "+$m%016lx:1#", (size_t)&value);
    platformMock_CommInitReceiveChecksummedData(packet, "+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$" MRI_ERROR_INVALID_ARGUMENT "#+"),
                   platformMock_CommGetTransmittedData() );
}

TEST(cmdMemory, MemoryRead_PacketBufferTooSmall_ShouldReturnOverflowResponse)
{
    uint8_t  value[16] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
    char     packet[64];
    snprintf(packet, sizeof(packet), "+$m%016lx,16#", (size_t)value);
    platformMock_CommInitReceiveChecksummedData(packet, "+$c#");
    platformMock_SetPacketBufferSize(4+16*2-1);
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$" MRI_ERROR_BUFFER_OVERRUN "#+"),
                   platformMock_CommGetTransmittedData() );
}

TEST(cmdMemory, MemoryRead64Unaligned)
{
    uint64_t value[2] = { 0x123456789abcdef0, 0x123456789abcdef0 };
    char     packet[64];
    snprintf(packet, sizeof(packet), "+$m%016lx,8#", ((size_t)value) + 4);
    platformMock_CommInitReceiveChecksummedData(packet, "+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$78563412f0debc9a#+"), platformMock_CommGetTransmittedData() );
}

TEST(cmdMemory, MemoryRead32Unaligned)
{
    uint32_t value[2] = { 0x12345678, 0x9abcdef0 };
    char     packet[64];
    snprintf(packet, sizeof(packet), "+$m%016lx,4#", ((size_t)value) + 2);
    platformMock_CommInitReceiveChecksummedData(packet, "+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$3412f0de#+"), platformMock_CommGetTransmittedData() );
}

TEST(cmdMemory, MemoryRead16Unaligned)
{
    uint16_t value[2] = { 0x1234, 0x5678 };
    char     packet[64];
    snprintf(packet, sizeof(packet), "+$m%016lx,2#", ((size_t)value) + 1);
    platformMock_CommInitReceiveChecksummedData(packet, "+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$1278#+"), platformMock_CommGetTransmittedData() );
}

TEST(cmdMemory, MemoryRead64_FaultAndReturnNoBytes)
{
    uint64_t value = 0x123456789abcdef0;
    char     packet[64];
    snprintf(packet, sizeof(packet), "+$m%016lx,8#", (size_t)&value);
    platformMock_CommInitReceiveChecksummedData(packet, "+$c#");
    platformMock_FaultOnSpecificMemoryCall(1);
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$E03#+"), platformMock_CommGetTransmittedData() );
}

TEST(cmdMemory, MemoryRead32_FaultAndReturnNoBytes)
{
    uint32_t value = 0x12345678;
    char     packet[64];
    snprintf(packet, sizeof(packet), "+$m%016lx,4#", (size_t)&value);
    platformMock_CommInitReceiveChecksummedData(packet, "+$c#");
    platformMock_FaultOnSpecificMemoryCall(1);
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$E03#+"), platformMock_CommGetTransmittedData() );
}

TEST(cmdMemory, MemoryRead16_FaultAndReturnNoBytes)
{
    uint16_t value = 0x1234;
    char     packet[64];
    snprintf(packet, sizeof(packet), "+$m%016lx,2#", (size_t)&value);
    platformMock_CommInitReceiveChecksummedData(packet, "+$c#");
    platformMock_FaultOnSpecificMemoryCall(1);
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$E03#+"), platformMock_CommGetTransmittedData() );
}

TEST(cmdMemory, MemoryRead8_FaultAndReturnNoBytes)
{
    uint8_t  value = 0x12;
    char     packet[64];
    snprintf(packet, sizeof(packet), "+$m%016lx,1#", (size_t)&value);
    platformMock_CommInitReceiveChecksummedData(packet, "+$c#");
    platformMock_FaultOnSpecificMemoryCall(1);
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$E03#+"), platformMock_CommGetTransmittedData() );
}

TEST(cmdMemory, MemoryRead8_FaultOnLastOfThreeBytes)
{
    uint8_t  values[3] = {0x12, 0x34, 0x56};
    char     packet[64];
    snprintf(packet, sizeof(packet), "+$m%016lx,3#", (size_t)values);
    platformMock_CommInitReceiveChecksummedData(packet, "+$c#");
    platformMock_FaultOnSpecificMemoryCall(3);
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$1234#+"), platformMock_CommGetTransmittedData() );
}



TEST(cmdMemory, MemoryWrite64Aligned)
{
    uint64_t value = -1;
    char     packet[64];
    snprintf(packet, sizeof(packet), "+$M%016lx,8:123456789abcdef0#", (size_t)&value);
    platformMock_CommInitReceiveChecksummedData(packet, "+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$OK#+"), platformMock_CommGetTransmittedData() );
    LONGS_EQUAL ( 0xf0debc9a78563412, value );
}

TEST(cmdMemory, MemoryWrite32Aligned)
{
    uint32_t value = 0xFFFFFFFF;
    char     packet[64];
    snprintf(packet, sizeof(packet), "+$M%016lx,4:12345678#", (size_t)&value);
    platformMock_CommInitReceiveChecksummedData(packet, "+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$OK#+"), platformMock_CommGetTransmittedData() );
    CHECK_EQUAL ( 0x78563412, value );
}

TEST(cmdMemory, MemoryWrite16Aligned)
{
    uint16_t value = 0xFFFF;
    char     packet[64];
    snprintf(packet, sizeof(packet), "+$M%016lx,2:1234#", (size_t)&value);
    platformMock_CommInitReceiveChecksummedData(packet, "+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$OK#+"), platformMock_CommGetTransmittedData() );
    CHECK_EQUAL ( 0x3412, value );
}

TEST(cmdMemory, MemoryWrite64_FaultOnFinalWrite)
{
    uint64_t value = -1;
    char     packet[64];
    snprintf(packet, sizeof(packet), "+$M%016lx,8:123456789abcdef0#", (size_t)&value);
    platformMock_CommInitReceiveChecksummedData(packet, "+$c#");
    platformMock_FaultOnSpecificMemoryCall(1);
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$" MRI_ERROR_MEMORY_ACCESS_FAILURE "#+"),
                   platformMock_CommGetTransmittedData() );
}

TEST(cmdMemory, MemoryWrite32_FaultOnFinalWrite)
{
    uint32_t value = 0xFFFFFFFF;
    char     packet[64];
    snprintf(packet, sizeof(packet), "+$M%016lx,4:12345678#", (size_t)&value);
    platformMock_CommInitReceiveChecksummedData(packet, "+$c#");
    platformMock_FaultOnSpecificMemoryCall(1);
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$" MRI_ERROR_MEMORY_ACCESS_FAILURE "#+"),
                   platformMock_CommGetTransmittedData() );
}

TEST(cmdMemory, MemoryWrite16_FaultOnFinalWrite)
{
    uint16_t value = 0xFFFF;
    char     packet[64];
    snprintf(packet, sizeof(packet), "+$M%016lx,2:1234#", (size_t)&value);
    platformMock_CommInitReceiveChecksummedData(packet, "+$c#");
    platformMock_FaultOnSpecificMemoryCall(1);
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$" MRI_ERROR_MEMORY_ACCESS_FAILURE "#+"),
                   platformMock_CommGetTransmittedData() );
 }


TEST(cmdMemory, MemoryWrite8)
{
    uint8_t  value = 0xFF;
    char     packet[64];
    snprintf(packet, sizeof(packet), "+$M%016lx,1:12#", (size_t)&value);
    platformMock_CommInitReceiveChecksummedData(packet, "+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$OK#+"), platformMock_CommGetTransmittedData() );
    CHECK_EQUAL ( 0x12, value );
}

TEST(cmdMemory, MemoryWrite8_InvalidParameterSeparator_ShouldReturnError)
{
    uint8_t  value = 0xFF;
    char     packet[64];
    snprintf(packet, sizeof(packet), "+$M%016lx,1,12#", (size_t)&value);
    platformMock_CommInitReceiveChecksummedData(packet, "+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$" MRI_ERROR_INVALID_ARGUMENT "#+"),
                   platformMock_CommGetTransmittedData() );
    CHECK_EQUAL ( 0xFF, value );
}

TEST(cmdMemory, MemoryWrite64Unaligned)
{
    uint64_t value[2] = { 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF };
    char     packet[64];
    snprintf(packet, sizeof(packet), "+$M%016lx,8:123456789abcdef0#", ((size_t)value) + 4);
    platformMock_CommInitReceiveChecksummedData(packet, "+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$OK#+"), platformMock_CommGetTransmittedData() );
    LONGS_EQUAL ( 0x78563412FFFFFFFF, value[0] );
    LONGS_EQUAL ( 0xFFFFFFFFf0debc9a, value[1] );
}

TEST(cmdMemory, MemoryWrite32Unaligned)
{
    uint32_t value[2] = { 0xFFFFFFFF, 0xFFFFFFFF };
    char     packet[64];
    snprintf(packet, sizeof(packet), "+$M%016lx,4:12345678#", ((size_t)value) + 2);
    platformMock_CommInitReceiveChecksummedData(packet, "+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$OK#+"), platformMock_CommGetTransmittedData() );
    CHECK_EQUAL ( 0x3412FFFF, value[0] );
    CHECK_EQUAL ( 0xFFFF7856, value[1] );
}

TEST(cmdMemory, MemoryWrite16Unaligned)
{
    uint16_t value[2] = { 0xFFFF, 0xFFFF };
    char     packet[64];
    snprintf(packet, sizeof(packet), "+$M%016lx,2:1234#", ((size_t)value) + 1);
    platformMock_CommInitReceiveChecksummedData(packet, "+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$OK#+"), platformMock_CommGetTransmittedData() );
    CHECK_EQUAL ( 0x12FF, value[0] );
    CHECK_EQUAL ( 0xFF34, value[1] );
}

TEST(cmdMemory, MemoryWrite64_TooFewBytesInPacket_ShouldReturnErrorAndNotModifyMemory)
{
    uint64_t value = -1;
    char     packet[64];
    snprintf(packet, sizeof(packet), "+$M%016lx,8:123456789abcde#", (size_t)&value);
    platformMock_CommInitReceiveChecksummedData(packet, "+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$" MRI_ERROR_BUFFER_OVERRUN "#+"),
                   platformMock_CommGetTransmittedData() );
    LONGS_EQUAL ( -1, value );
}

TEST(cmdMemory, MemoryWrite32_TooFewBytesInPacket_ShouldReturnErrorAndNotModifyMemory)
{
    uint32_t value = 0xFFFFFFFF;
    char     packet[64];
    snprintf(packet, sizeof(packet), "+$M%016lx,4:123456#", (size_t)&value);
    platformMock_CommInitReceiveChecksummedData(packet, "+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$" MRI_ERROR_BUFFER_OVERRUN "#+"),
                   platformMock_CommGetTransmittedData() );
    CHECK_EQUAL ( 0xFFFFFFFF, value );
}

TEST(cmdMemory, MemoryWrite16_TooFewBytesInPacket_ShouldReturnErrorAndNotModifyMemory)
{
    uint16_t value = 0xFFFF;
    char     packet[64];
    snprintf(packet, sizeof(packet), "+$M%016lx,2:12#", (size_t)&value);
    platformMock_CommInitReceiveChecksummedData(packet, "+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$" MRI_ERROR_BUFFER_OVERRUN "#+"),
                   platformMock_CommGetTransmittedData() );
    CHECK_EQUAL ( 0xFFFF, value );
}

TEST(cmdMemory, MemoryWrite8_TooFewBytesInPacket_ShouldReturnErrorAndNotModifyMemory)
{
    uint8_t  value = 0xFF;
    char     packet[64];
    snprintf(packet, sizeof(packet), "+$M%016lx,1:#", (size_t)&value);
    platformMock_CommInitReceiveChecksummedData(packet, "+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$" MRI_ERROR_BUFFER_OVERRUN "#+"),
                   platformMock_CommGetTransmittedData() );
    CHECK_EQUAL ( 0xFF, value );
}

TEST(cmdMemory, MemoryWrite8_FaultOnFirstWrite)
{
    uint8_t value = 0xFF;
    char     packet[64];
    snprintf(packet, sizeof(packet), "+$M%016lx,1:12#", (size_t)&value);
    platformMock_CommInitReceiveChecksummedData(packet, "+$c#");
    platformMock_FaultOnSpecificMemoryCall(1);
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$" MRI_ERROR_MEMORY_ACCESS_FAILURE "#+"),
                   platformMock_CommGetTransmittedData() );
}

TEST(cmdMemory, MemoryWrite8_FaultOnSecondWrite)
{
    uint8_t value[3] = {0xFF, 0xFF, 0xFF};
    char     packet[64];
    snprintf(packet, sizeof(packet), "+$M%016lx,3:010203#", (size_t)&value);
    platformMock_CommInitReceiveChecksummedData(packet, "+$c#");
    platformMock_FaultOnSpecificMemoryCall(2);
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$" MRI_ERROR_MEMORY_ACCESS_FAILURE "#+"),
                   platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 1, value[0]);
    // This next write made it through since the mocked fault is only checked after actual native write.
    CHECK_EQUAL( 2, value[1]);
    // It shouldn't make it to this next memory write though.
    CHECK_EQUAL( 0xFF, value[2]);
}



TEST(cmdMemory, BinaryMemoryWrite64Aligned)
{
    uint64_t value = -1;
    char     packet[64];
    snprintf(packet, sizeof(packet), "+$X%016lx,8:\x12\x34\x56\x78\x9a\xbc\xde\xf0#", (size_t)&value);
    platformMock_CommInitReceiveChecksummedData(packet, "+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$OK#+"), platformMock_CommGetTransmittedData() );
    CHECK_EQUAL ( 1, platformMock_GetSyncICacheToDCacheCalls() );
    LONGS_EQUAL ( 0xf0debc9a78563412, value );
}

TEST(cmdMemory, BinaryMemoryWrite32Aligned)
{
    uint32_t value = 0xFFFFFFFF;
    char     packet[64];
    snprintf(packet, sizeof(packet), "+$X%016lx,4:\x12\x34\x56\x78#", (size_t)&value);
    platformMock_CommInitReceiveChecksummedData(packet, "+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$OK#+"), platformMock_CommGetTransmittedData() );
    CHECK_EQUAL ( 1, platformMock_GetSyncICacheToDCacheCalls() );
    CHECK_EQUAL ( 0x78563412, value );
}

TEST(cmdMemory, BinaryMemoryWrite16Aligned)
{
    uint16_t value = 0xFFFF;
    char     packet[64];
    snprintf(packet, sizeof(packet), "+$X%016lx,2:\x12\x34#", (size_t)&value);
    platformMock_CommInitReceiveChecksummedData(packet, "+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$OK#+"), platformMock_CommGetTransmittedData() );
    CHECK_EQUAL ( 1, platformMock_GetSyncICacheToDCacheCalls() );
    CHECK_EQUAL ( 0x3412, value );
}

TEST(cmdMemory, BinaryMemoryWrite64_FaultOnFinalWrite)
{
    uint64_t value = -1;
    char     packet[64];
    snprintf(packet, sizeof(packet), "+$X%016lx,8:\x12\x34\x56\x78\x9a\xbc\xde\xf0#", (size_t)&value);
    platformMock_CommInitReceiveChecksummedData(packet, "+$c#");
    platformMock_FaultOnSpecificMemoryCall(1);
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$" MRI_ERROR_MEMORY_ACCESS_FAILURE "#+"),
                   platformMock_CommGetTransmittedData() );
    CHECK_EQUAL ( 0, platformMock_GetSyncICacheToDCacheCalls() );
}

TEST(cmdMemory, BinaryMemoryWrite32_FaultOnFinalWrite)
{
    uint32_t value = 0xFFFFFFFF;
    char     packet[64];
    snprintf(packet, sizeof(packet), "+$X%016lx,4:\x12\x34\x56\x78#", (size_t)&value);
    platformMock_CommInitReceiveChecksummedData(packet, "+$c#");
    platformMock_FaultOnSpecificMemoryCall(1);
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$" MRI_ERROR_MEMORY_ACCESS_FAILURE "#+"),
                   platformMock_CommGetTransmittedData() );
    CHECK_EQUAL ( 0, platformMock_GetSyncICacheToDCacheCalls() );
}

TEST(cmdMemory, BinaryMemoryWrite16_FaultOnFinalWrite)
{
    uint16_t value = 0xFFFF;
    char     packet[64];
    snprintf(packet, sizeof(packet), "+$X%016lx,2:\x12\x34#", (size_t)&value);
    platformMock_CommInitReceiveChecksummedData(packet, "+$c#");
    platformMock_FaultOnSpecificMemoryCall(1);
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$" MRI_ERROR_MEMORY_ACCESS_FAILURE "#+"),
                   platformMock_CommGetTransmittedData() );
    CHECK_EQUAL ( 0, platformMock_GetSyncICacheToDCacheCalls() );
}

TEST(cmdMemory, BinaryMemoryWrite8)
{
    uint8_t  value = 0xFF;
    char     packet[64];
    snprintf(packet, sizeof(packet), "+$X%016lx,1:\x12#", (size_t)&value);
    platformMock_CommInitReceiveChecksummedData(packet, "+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$OK#+"), platformMock_CommGetTransmittedData() );
    CHECK_EQUAL ( 1, platformMock_GetSyncICacheToDCacheCalls() );
    CHECK_EQUAL ( 0x12, value );
}

TEST(cmdMemory, BinaryMemoryWrite8_InvalidParameterSeparator_ShouldReturnError)
{
    uint8_t  value = 0xFF;
    char     packet[64];
    snprintf(packet, sizeof(packet), "+$X%016lx,1,\x12#", (size_t)&value);
    platformMock_CommInitReceiveChecksummedData(packet, "+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$" MRI_ERROR_INVALID_ARGUMENT "#+"),
                   platformMock_CommGetTransmittedData() );
    CHECK_EQUAL ( 0, platformMock_GetSyncICacheToDCacheCalls() );
    CHECK_EQUAL ( 0xFF, value );
}

TEST(cmdMemory, BinaryMemoryWrite64Unaligned)
{
    uint64_t value[2] = { -1, -1 };
    char     packet[64];
    snprintf(packet, sizeof(packet), "+$X%016lx,8:\x12\x34\x56\x78\x9a\xbc\xde\xf0#", ((size_t)value) + 4);
    platformMock_CommInitReceiveChecksummedData(packet, "+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$OK#+"), platformMock_CommGetTransmittedData() );
    CHECK_EQUAL ( 1, platformMock_GetSyncICacheToDCacheCalls() );
    LONGS_EQUAL ( 0x78563412FFFFFFFF, value[0] );
    LONGS_EQUAL ( 0xFFFFFFFFF0DEBC9A, value[1] );
}

TEST(cmdMemory, BinaryMemoryWrite32Unaligned)
{
    uint32_t value[2] = { 0xFFFFFFFF, 0xFFFFFFFF };
    char     packet[64];
    snprintf(packet, sizeof(packet), "+$X%016lx,4:\x12\x34\x56\x78#", ((size_t)value) + 2);
    platformMock_CommInitReceiveChecksummedData(packet, "+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$OK#+"), platformMock_CommGetTransmittedData() );
    CHECK_EQUAL ( 1, platformMock_GetSyncICacheToDCacheCalls() );
    CHECK_EQUAL ( 0x3412FFFF, value[0] );
    CHECK_EQUAL ( 0xFFFF7856, value[1] );
}

TEST(cmdMemory, BinaryMemoryWrite16Unaligned)
{
    uint16_t value[2] = { 0xFFFF, 0xFFFF };
    char     packet[64];
    snprintf(packet, sizeof(packet), "+$X%016lx,2:\x12\x34#", ((size_t)value) + 1);
    platformMock_CommInitReceiveChecksummedData(packet, "+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$OK#+"), platformMock_CommGetTransmittedData() );
    CHECK_EQUAL ( 1, platformMock_GetSyncICacheToDCacheCalls() );
    CHECK_EQUAL ( 0x12FF, value[0] );
    CHECK_EQUAL ( 0xFF34, value[1] );
}

TEST(cmdMemory, BinaryMemoryWrite64_TooFewBytesInPacket_ShouldReturnErrorAndNotModifyMemory)
{
    uint64_t value = -1;
    char     packet[64];
    snprintf(packet, sizeof(packet), "+$X%016lx,8:\x12\x34\x56\x78\x9a\xbc\xde#", (size_t)&value);
    platformMock_CommInitReceiveChecksummedData(packet, "+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$" MRI_ERROR_BUFFER_OVERRUN "#+"),
                   platformMock_CommGetTransmittedData() );
    CHECK_EQUAL ( 0, platformMock_GetSyncICacheToDCacheCalls() );
    LONGS_EQUAL ( -1, value );
}

TEST(cmdMemory, BinaryMemoryWrite32_TooFewBytesInPacket_ShouldReturnErrorAndNotModifyMemory)
{
    uint32_t value = 0xFFFFFFFF;
    char     packet[64];
    snprintf(packet, sizeof(packet), "+$X%016lx,4:\x12\x34\x56#", (size_t)&value);
    platformMock_CommInitReceiveChecksummedData(packet, "+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$" MRI_ERROR_BUFFER_OVERRUN "#+"),
                   platformMock_CommGetTransmittedData() );
    CHECK_EQUAL ( 0, platformMock_GetSyncICacheToDCacheCalls() );
    CHECK_EQUAL ( 0xFFFFFFFF, value );
}

TEST(cmdMemory, BinaryMemoryWrite16_TooFewBytesInPacket_ShouldReturnErrorAndNotModifyMemory)
{
    uint16_t value = 0xFFFF;
    char     packet[64];
    snprintf(packet, sizeof(packet), "+$X%016lx,2:\x12#", (size_t)&value);
    platformMock_CommInitReceiveChecksummedData(packet, "+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$" MRI_ERROR_BUFFER_OVERRUN "#+"),
                   platformMock_CommGetTransmittedData() );
    CHECK_EQUAL ( 0, platformMock_GetSyncICacheToDCacheCalls() );
    CHECK_EQUAL ( 0xFFFF, value );
}

TEST(cmdMemory, BinaryMemoryWrite8_TooFewBytesInPacket_ShouldReturnErrorAndNotModifyMemory)
{
    uint8_t  value = 0xFF;
    char     packet[64];
    snprintf(packet, sizeof(packet), "+$X%016lx,1:#", (size_t)&value);
    platformMock_CommInitReceiveChecksummedData(packet, "+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$" MRI_ERROR_BUFFER_OVERRUN "#+"),
                   platformMock_CommGetTransmittedData() );
    CHECK_EQUAL ( 0, platformMock_GetSyncICacheToDCacheCalls() );
    CHECK_EQUAL ( 0xFF, value );
}

TEST(cmdMemory, BinaryMemoryWrite8_FaultOnFirstWrite)
{
    uint8_t value = 0xFF;
    char     packet[64];
    snprintf(packet, sizeof(packet), "+$X%016lx,1:\x12#", (size_t)&value);
    platformMock_CommInitReceiveChecksummedData(packet, "+$c#");
    platformMock_FaultOnSpecificMemoryCall(1);
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$" MRI_ERROR_MEMORY_ACCESS_FAILURE "#+"),
                   platformMock_CommGetTransmittedData() );
    CHECK_EQUAL ( 0, platformMock_GetSyncICacheToDCacheCalls() );
}

TEST(cmdMemory, BinaryMemoryWrite8_FaultOnSecondWrite)
{
    uint8_t value[3] = {0xFF, 0xFF, 0xFF};
    char     packet[64];
    snprintf(packet, sizeof(packet), "+$X%016lx,3:\x01\x02\x03#", (size_t)&value);
    platformMock_CommInitReceiveChecksummedData(packet, "+$c#");
    platformMock_FaultOnSpecificMemoryCall(2);
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$" MRI_ERROR_MEMORY_ACCESS_FAILURE "#+"),
                   platformMock_CommGetTransmittedData() );
    CHECK_EQUAL( 1, value[0]);
    // This next write made it through since the mocked fault is only checked after actual native write.
    CHECK_EQUAL( 2, value[1]);
    // It shouldn't make it to this next memory write though.
    CHECK_EQUAL( 0xFF, value[2]);
    CHECK_EQUAL ( 0, platformMock_GetSyncICacheToDCacheCalls() );
}

TEST(cmdMemory, BinaryMemoryWrite8_UseEscapedByte)
{
    uint8_t  value = 0xFF;
    char     packet[64];
    snprintf(packet, sizeof(packet), "+$X%016lx,1:}%c#", (size_t)&value, '}' ^ 0x20);
    platformMock_CommInitReceiveChecksummedData(packet, "+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$OK#+"), platformMock_CommGetTransmittedData() );
    CHECK_EQUAL ( '}', value );
    CHECK_EQUAL ( 1, platformMock_GetSyncICacheToDCacheCalls() );
}

TEST(cmdMemory, BinaryMemoryWrite8_BufferTooSmallForEscapedByte_ReturnErrorResponse)
{
    uint8_t  value = 0xFF;
    char     packet[64];
    snprintf(packet, sizeof(packet), "+$X%016lx,1:}#", (size_t)&value);
    platformMock_CommInitReceiveChecksummedData(packet, "+$c#");
        mriDebugException(platformMock_GetContext());
    STRCMP_EQUAL ( platformMock_CommChecksumData("$T05responseT#+$" MRI_ERROR_BUFFER_OVERRUN "#+"),
                   platformMock_CommGetTransmittedData() );
    CHECK_EQUAL ( 0, platformMock_GetSyncICacheToDCacheCalls() );
    CHECK_EQUAL ( 0xFF, value );
}
