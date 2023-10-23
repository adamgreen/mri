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
#include <string.h>
#include <limits.h>

extern "C"
{
#include <core/context.h>
#include <core/try_catch.h>
}

// Include C++ headers for test harness.
#include "CppUTest/TestHarness.h"

TEST_GROUP(MriContext)
{
    void setup()
    {
        clearExceptionCode();
    }

    void teardown()
    {
        LONGS_EQUAL( 0, getExceptionCode() );
        clearExceptionCode();
    }
};

TEST(MriContext, ContextInit_SingleItem)
{
    uintmri_t value = 0xBAADFEED;
    ContextSection entries[] = { {.pValues = &value, .count = 1} };
    MriContext context;

    Context_Init(&context, entries, sizeof(entries)/sizeof(entries[0]));
    LONGS_EQUAL( 1, Context_Count(&context) );
    LONGS_EQUAL( 0xBAADFEED, Context_Get(&context, 0) );
}

TEST(MriContext, ContextSet_SingleItem)
{
    uintmri_t value = 0xBAADFEED;
    ContextSection entries[] = { {.pValues = &value, .count = 1} };
    MriContext context;

    Context_Init(&context, entries, sizeof(entries)/sizeof(entries[0]));
    LONGS_EQUAL( 1, Context_Count(&context) );
    LONGS_EQUAL( 0xBAADFEED, Context_Get(&context, 0) );

    Context_Set(&context, 0, 0x5A5A5A5A);
    LONGS_EQUAL( 0x5A5A5A5A, Context_Get(&context, 0) );
}

TEST(MriContext, Context_TwoItem_OneEntry)
{
    uintmri_t values[] = { 0xBAADFEED, 0x5A5A5A5A };
    ContextSection entries[] = { {.pValues = values, .count = 2} };
    MriContext context;

    Context_Init(&context, entries, sizeof(entries)/sizeof(entries[0]));
    LONGS_EQUAL( 2, Context_Count(&context) );
    LONGS_EQUAL( 0xBAADFEED, Context_Get(&context, 0) );
    LONGS_EQUAL( 0x5A5A5A5A, Context_Get(&context, 1) );
}

TEST(MriContext, Context_TwoItem_TwoEntries)
{
    uintmri_t value0 = 0xBAADFEED;
    uintmri_t value1 = 0x5A5A5A5A;
    ContextSection entries[] = { {.pValues = &value0, .count = 1},  {.pValues = &value1, .count = 1}};
    MriContext context;

    Context_Init(&context, entries, sizeof(entries)/sizeof(entries[0]));
    LONGS_EQUAL( 2, Context_Count(&context) );
    LONGS_EQUAL( 0xBAADFEED, Context_Get(&context, 0) );
    LONGS_EQUAL( 0x5A5A5A5A, Context_Get(&context, 1) );
}

TEST(MriContext, ContextGet_InvalidIndex_ShouldThrowAndReturn0)
{
    uintmri_t value = 0xBAADFEED;
    ContextSection entries[] = { {.pValues = &value, .count = 1} };
    MriContext context;

    Context_Init(&context, entries, sizeof(entries)/sizeof(entries[0]));
    LONGS_EQUAL( 0, Context_Get(&context, 1) );
    LONGS_EQUAL( bufferOverrunException, getExceptionCode() );
    clearExceptionCode();
}

TEST(MriContext, ContextSet_InvalidIndex_ShouldThrow)
{
    uintmri_t value = 0xBAADFEED;
    ContextSection entries[] = { {.pValues = &value, .count = 1} };
    MriContext context;

    Context_Init(&context, entries, sizeof(entries)/sizeof(entries[0]));
    Context_Set(&context, 1, 0x5A5A5A5A);
    LONGS_EQUAL( bufferOverrunException, getExceptionCode() );
    clearExceptionCode();
}

TEST(MriContext, Context_CopyToBuffer)
{
    uintmri_t value0 = 0x1100223344556677;
    uintmri_t value1 = 0x8899AABBCCDDEEFF;
    ContextSection entries[] = { {.pValues = &value0, .count = 1},  {.pValues = &value1, .count = 1}};
    MriContext context;
    Context_Init(&context, entries, sizeof(entries)/sizeof(entries[0]));

    char   bufferData[2*sizeof(uintmri_t)*2+1];
    Buffer buffer;
    memset(bufferData, 0xFF, sizeof(bufferData));
    Buffer_Init(&buffer, bufferData, sizeof(bufferData)-1);

    Context_CopyToBuffer(&context, &buffer);
    bufferData[sizeof(bufferData)-1] = '\0';
    STRCMP_EQUAL ( "7766554433220011ffeeddccbbaa9988", bufferData );
}

TEST(MriContext, Context_CopyFromBuffer)
{
    uintmri_t value0 = -1;
    uintmri_t value1 = -1;
    ContextSection entries[] = { {.pValues = &value0, .count = 1},  {.pValues = &value1, .count = 1}};
    MriContext context;
    Context_Init(&context, entries, sizeof(entries)/sizeof(entries[0]));

    char   bufferData[] = "7766554433220011FFEEDDCCBBAA9988";
    Buffer buffer;
    Buffer_Init(&buffer, bufferData, sizeof(bufferData)-1);

    Context_CopyFromBuffer(&context, &buffer);
    LONGS_EQUAL(0x1100223344556677, value0);
    LONGS_EQUAL(0x8899AABBCCDDEEFF, value1);
}
