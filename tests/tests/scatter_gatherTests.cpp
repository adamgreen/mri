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
#include <string.h>
#include <limits.h>

extern "C"
{
#include <core/scatter_gather.h>
#include <core/try_catch.h>
}

// Include C++ headers for test harness.
#include "CppUTest/TestHarness.h"

TEST_GROUP(ScatterGather)
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

TEST(ScatterGather, ScatterGatherInit_SingleItem)
{
    uint32_t value = 0xBAADFEED;
    ScatterGatherEntry entries[] = { {.pValues = &value, .count = 1} };
    ScatterGather scatterGather;

    ScatterGather_Init(&scatterGather, entries, sizeof(entries)/sizeof(entries[0]));
    LONGS_EQUAL( 1, ScatterGather_Count(&scatterGather) );
    LONGS_EQUAL( 0xBAADFEED, ScatterGather_Get(&scatterGather, 0) );
}

TEST(ScatterGather, ScatterGatherSet_SingleItem)
{
    uint32_t value = 0xBAADFEED;
    ScatterGatherEntry entries[] = { {.pValues = &value, .count = 1} };
    ScatterGather scatterGather;

    ScatterGather_Init(&scatterGather, entries, sizeof(entries)/sizeof(entries[0]));
    LONGS_EQUAL( 1, ScatterGather_Count(&scatterGather) );
    LONGS_EQUAL( 0xBAADFEED, ScatterGather_Get(&scatterGather, 0) );

    ScatterGather_Set(&scatterGather, 0, 0x5A5A5A5A);
    LONGS_EQUAL( 0x5A5A5A5A, ScatterGather_Get(&scatterGather, 0) );
}

TEST(ScatterGather, ScatterGather_TwoItem_OneEntry)
{
    uint32_t values[] = { 0xBAADFEED, 0x5A5A5A5A };
    ScatterGatherEntry entries[] = { {.pValues = values, .count = 2} };
    ScatterGather scatterGather;

    ScatterGather_Init(&scatterGather, entries, sizeof(entries)/sizeof(entries[0]));
    LONGS_EQUAL( 2, ScatterGather_Count(&scatterGather) );
    LONGS_EQUAL( 0xBAADFEED, ScatterGather_Get(&scatterGather, 0) );
    LONGS_EQUAL( 0x5A5A5A5A, ScatterGather_Get(&scatterGather, 1) );
}

TEST(ScatterGather, ScatterGather_TwoItem_TwoEntries)
{
    uint32_t value0 = 0xBAADFEED;
    uint32_t value1 = 0x5A5A5A5A;
    ScatterGatherEntry entries[] = { {.pValues = &value0, .count = 1},  {.pValues = &value1, .count = 1}};
    ScatterGather scatterGather;

    ScatterGather_Init(&scatterGather, entries, sizeof(entries)/sizeof(entries[0]));
    LONGS_EQUAL( 2, ScatterGather_Count(&scatterGather) );
    LONGS_EQUAL( 0xBAADFEED, ScatterGather_Get(&scatterGather, 0) );
    LONGS_EQUAL( 0x5A5A5A5A, ScatterGather_Get(&scatterGather, 1) );
}

TEST(ScatterGather, ScatterGatherGet_InvalidIndex_ShouldThrowAndReturn0)
{
    uint32_t value = 0xBAADFEED;
    ScatterGatherEntry entries[] = { {.pValues = &value, .count = 1} };
    ScatterGather scatterGather;

    ScatterGather_Init(&scatterGather, entries, sizeof(entries)/sizeof(entries[0]));
    LONGS_EQUAL( 0, ScatterGather_Get(&scatterGather, 1) );
    LONGS_EQUAL( bufferOverrunException, getExceptionCode() );
    clearExceptionCode();
}

TEST(ScatterGather, ScatterGatherSet_InvalidIndex_ShouldThrow)
{
    uint32_t value = 0xBAADFEED;
    ScatterGatherEntry entries[] = { {.pValues = &value, .count = 1} };
    ScatterGather scatterGather;

    ScatterGather_Init(&scatterGather, entries, sizeof(entries)/sizeof(entries[0]));
    ScatterGather_Set(&scatterGather, 1, 0x5A5A5A5A);
    LONGS_EQUAL( bufferOverrunException, getExceptionCode() );
    clearExceptionCode();
}
