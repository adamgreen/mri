/* Copyright 2022 Adam Green (https://github.com/adamgreen/)

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


// Can set the following to true to run the tests using real Standard C Library versions of these functions to see
// if they pass the same way. This verifies the tests themselves.
#define USE_REAL_STD_C_LIB_VERSIONS false

#if !USE_REAL_STD_C_LIB_VERSIONS
    extern "C"
    {
        #include <core/libc.h>
    }
#else
    #define mri_memcpy memcpy
    #define mri_memset memset
    #define mri_strcmp strcmp
    #define mri_strncmp strncmp
    #define mri_strlen strlen
    #define mri_strstr strstr
    #define mri_memmove memmove
#endif // !USE_REAL_STD_C_LIB_VERSIONS


// Include C++ headers for test harness.
#include "CppUTest/TestHarness.h"

TEST_GROUP(libc)
{
    uint8_t m_srcBuffer[16];
    uint8_t m_destBuffer[16];
    uint8_t m_allOnes[16];
    uint8_t m_allZeroes[16];

    void setup()
    {
        memset(m_srcBuffer, 0x00, sizeof(m_srcBuffer));
        memset(m_allZeroes, 0x00, sizeof(m_allZeroes));
        memset(m_destBuffer, 0xff, sizeof(m_destBuffer));
        memset(m_allOnes, 0xff, sizeof(m_allOnes));
    }

    void teardown()
    {
    }

    void fillBuferWithAscendingValues(uint8_t* pBuff, size_t len)
    {
        for (size_t i = 0 ; i < len ; i++)
        {
            *pBuff++ = (uint8_t)i;
        }
    }
};

TEST(libc, mri_memcpy_FullCopy)
{
    void* pReturn = mri_memcpy(m_destBuffer, m_srcBuffer, sizeof(m_destBuffer));
    POINTERS_EQUAL(pReturn, m_destBuffer);
    LONGS_EQUAL(0, memcmp(m_destBuffer, m_srcBuffer, sizeof(m_destBuffer)));
}

TEST(libc, mri_memcpy_HalfCopy)
{
    void* pReturn = mri_memcpy(m_destBuffer, m_srcBuffer, sizeof(m_destBuffer)/2);
    POINTERS_EQUAL(pReturn, m_destBuffer);
    LONGS_EQUAL(0, memcmp(m_destBuffer, m_allZeroes, sizeof(m_destBuffer)/2));
    LONGS_EQUAL(0, memcmp(&m_destBuffer[sizeof(m_destBuffer)/2], m_allOnes, sizeof(m_destBuffer)/2));
}



TEST(libc, mri_memset_AllToZeroes)
{
    void* pReturn = mri_memset(m_destBuffer, 0x00, sizeof(m_destBuffer));
    POINTERS_EQUAL(pReturn, m_destBuffer);
    LONGS_EQUAL(0, memcmp(m_destBuffer, m_allZeroes, sizeof(m_destBuffer)));
}

TEST(libc, mri_memset_HalfSet)
{
    void* pReturn = mri_memset(m_destBuffer, 0x00, sizeof(m_destBuffer)/2);
    POINTERS_EQUAL(pReturn, m_destBuffer);
    LONGS_EQUAL(0, memcmp(m_destBuffer, m_allZeroes, sizeof(m_destBuffer)/2));
    LONGS_EQUAL(0, memcmp(&m_destBuffer[sizeof(m_destBuffer)/2], m_allOnes, sizeof(m_destBuffer)/2));
}



TEST(libc, mri_strcmp)
{
    CHECK_TRUE(mri_strcmp("", "") == 0);

    CHECK_TRUE(mri_strcmp("a", "a") == 0);
    CHECK_TRUE(mri_strcmp("a", "b") < 0);
    CHECK_TRUE(mri_strcmp("a", "c") < 0);

    CHECK_TRUE(mri_strcmp("", "a") < 0);
    CHECK_TRUE(mri_strcmp("a", "") > 0);

    CHECK_TRUE(mri_strcmp("\x01", "\xff") < 0);
    CHECK_TRUE(mri_strcmp("b", "a") > 0);
    CHECK_TRUE(mri_strcmp("a", "aa") < 0);
    CHECK_TRUE(mri_strcmp("aa", "a") > 0);
}



TEST(libc, mri_strncmp)
{
    CHECK_TRUE(mri_strncmp("", "", 0) == 0);
    CHECK_TRUE(mri_strncmp("", "", 1) == 0);

    CHECK_TRUE(mri_strncmp("a", "a", 1) == 0);
    CHECK_TRUE(mri_strncmp("a", "b", 1) < 0);
    CHECK_TRUE(mri_strncmp("a", "c", 1) < 0);
    CHECK_TRUE(mri_strncmp("a", "a", 2) == 0);
    CHECK_TRUE(mri_strncmp("a", "b", 2) < 0);
    CHECK_TRUE(mri_strncmp("a", "c", 2) < 0);

    CHECK_TRUE(mri_strncmp("abc", "ab", 2) == 0);
    CHECK_TRUE(mri_strncmp("ab", "abc", 2) == 0);

    CHECK_TRUE(mri_strncmp("", "ab", 0) == 0);

    CHECK_TRUE(mri_strncmp("", "a", 1) < 0);
    CHECK_TRUE(mri_strncmp("a", "", 1) > 0);
    CHECK_TRUE(mri_strncmp("", "a", 2) < 0);
    CHECK_TRUE(mri_strncmp("a", "", 2) > 0);

    CHECK_TRUE(mri_strncmp("\x01", "\xff", 1) < 0);
    CHECK_TRUE(mri_strncmp("\x01", "\xff", 2) < 0);
    CHECK_TRUE(mri_strncmp("b", "a", 1) > 0);
    CHECK_TRUE(mri_strncmp("a", "aa", 2) < 0);
    CHECK_TRUE(mri_strncmp("aa", "a", 2) > 0);
    CHECK_TRUE(mri_strncmp("a", "aa", 1) == 0);
    CHECK_TRUE(mri_strncmp("aa", "a", 1) == 0);
}



TEST(libc, mri_strlen)
{
    LONGS_EQUAL(0, mri_strlen(""));
    LONGS_EQUAL(1, mri_strlen("a"));
}



TEST(libc, mri_strstr)
{
    const char* haystack = "haystack";

    CHECK_TRUE(mri_strstr(haystack, "") == haystack);
    CHECK_TRUE(mri_strstr(haystack, "needle") == NULL);
    CHECK_TRUE(mri_strstr(haystack, "haystack") == haystack);
    CHECK_TRUE(mri_strstr(haystack, "haystac") == haystack);
    CHECK_TRUE(mri_strstr(haystack, "k") == &haystack[7]);
    CHECK_TRUE(mri_strstr(haystack, "haystacl") == NULL);
}



TEST(libc, mri_memmove_NonOverlappingCopy)
{
    void* pReturn = mri_memmove(m_destBuffer, m_srcBuffer, sizeof(m_destBuffer));
    POINTERS_EQUAL(pReturn, m_destBuffer);
    LONGS_EQUAL(0, memcmp(m_destBuffer, m_srcBuffer, sizeof(m_destBuffer)));
}

TEST(libc, mri_memmove_OverlappingCopy_DestLowerThanSource)
{
    uint8_t buffer[64];

    fillBuferWithAscendingValues(buffer, sizeof(buffer));
    void* pReturn = mri_memmove(&buffer[0], &buffer[1], 63);
    POINTERS_EQUAL(pReturn, &buffer[0]);
    BYTES_EQUAL(1, buffer[0]);
    BYTES_EQUAL(2, buffer[1]);
    BYTES_EQUAL(63, buffer[62]);
    BYTES_EQUAL(63, buffer[63]); // This byte should be untouched since data was moved down.
}

TEST(libc, mri_memmove_OverlappingCopy_DestHigherThanSource)
{
    uint8_t buffer[64];

    fillBuferWithAscendingValues(buffer, sizeof(buffer));
    void* pReturn = mri_memmove(&buffer[1], &buffer[0], 63);
    POINTERS_EQUAL(pReturn, &buffer[1]);
    BYTES_EQUAL(0, buffer[0]); // This byte should be untouched since data was moved up.
    BYTES_EQUAL(0, buffer[1]);
    BYTES_EQUAL(61, buffer[62]);
    BYTES_EQUAL(62, buffer[63]);
}

TEST(libc, mri_memmove_OverlappingCopy_DestHigherThanSource_Only1ByteOverlapping)
{
    uint8_t buffer[128];

    fillBuferWithAscendingValues(buffer, sizeof(buffer));
    void* pReturn = mri_memmove(&buffer[63], &buffer[0], 64);
    POINTERS_EQUAL(pReturn, &buffer[63]);
    BYTES_EQUAL(0, buffer[63]);
    BYTES_EQUAL(1, buffer[64]);
    BYTES_EQUAL(63, buffer[63+63]);
}
