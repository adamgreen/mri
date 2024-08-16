/* Copyright 2024 Adam Green (https://github.com/adamgreen/)

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
#include <core/platforms.h>
}

// Include C++ headers for test harness.
#include "CppUTest/TestHarness.h"

TEST_GROUP(NativeMem)
{
    void setup()
    {
    }

    void teardown()
    {
    }
};

TEST(NativeMem, Platform_MemoryRead)
{
    const uint8_t srcBuffer[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };
    uint8_t       destBuffer[8] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

    uintmri_t bytesRead = Platform_ReadMemory(destBuffer, (uintmri_t)&srcBuffer[0], sizeof(destBuffer));
    CHECK_EQUAL(sizeof(destBuffer), bytesRead);
    CHECK_EQUAL(0, memcmp(destBuffer, srcBuffer, sizeof(destBuffer)));
}
