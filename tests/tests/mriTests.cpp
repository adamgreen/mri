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
#include <core.h>
#include <token.h>
}
#include <platformMock.h>

// Include C++ headers for test harness.
#include "CppUTest/TestHarness.h"


TEST_GROUP(Mri)
{
    int     m_expectException;            
    
    void setup()
    {
        m_expectException = noException;
        platformMock_Init();
    }

    void teardown()
    {
        LONGS_EQUAL ( m_expectException, getExceptionCode() );
        clearExceptionCode();
    }
    
    void validateExceptionCode(int expectedExceptionCode)
    {
        m_expectException = expectedExceptionCode;
        LONGS_EQUAL ( expectedExceptionCode, getExceptionCode() );
    }
};

TEST(Mri, __mriInit_MakeSureThatItCallsMriPlatformInit)
{
    __mriInit("MRI_UART_MBED_USB");
    LONGS_EQUAL( 1, platformMock_GetInitCount() );
}

TEST(Mri, __mriInit_MakeSureThatItPassesTokenizedStringIntoMriPlatformInit)
{
    __mriInit("MRI_UART_MBED_USB MRI_UART_SHARE");
    
    Token* pInitTokens = platformMock_GetInitTokenCopy();
    LONGS_EQUAL( 2, Token_GetTokenCount(pInitTokens) );
    STRCMP_EQUAL( "MRI_UART_MBED_USB", Token_GetToken(pInitTokens, 0) );
    STRCMP_EQUAL( "MRI_UART_SHARE", Token_GetToken(pInitTokens, 1) );
}

TEST(Mri, __mriInit_MakeSureThatItSetsProperFlagsOnSuccessfulInit)
{
    __mriInit("MRI_UART_MBED_USB");
    CHECK_TRUE( IsFirstException() );
    CHECK_TRUE( WasSuccessfullyInit() );
}

TEST(Mri, __mriInit_HandlesMriPlatformInitThrowingException)
{
    platformMock_SetInitException(timeoutException);
    __mriInit("MRI_UART_MBED_USB");
    validateExceptionCode(timeoutException);
    CHECK_FALSE( IsFirstException() );
    CHECK_FALSE( WasSuccessfullyInit() );
}
