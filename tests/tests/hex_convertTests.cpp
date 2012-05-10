/* Copyright 2012 Adam Green (http://mbed.org/users/AdamGreen/)

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
#include "hex_convert.h"
}

// Include C++ headers for test harness.
#include "CppUTest/TestHarness.h"

TEST_GROUP(HexConvert)
{
    void setup()
    {
    }

    void teardown()
    {
    }
};

TEST(HexConvert, ExtractLowNibble)
{
    uint8_t value = 0xaf;
    
    LONGS_EQUAL( 0xf, EXTRACT_LO_NIBBLE(value) );
}

TEST(HexConvert, ExtractHighNibble)
{
    uint8_t value = 0xfa;
    
    LONGS_EQUAL( 0xf, EXTRACT_HI_NIBBLE(value) );
}

TEST(HexConvert, NibbleToHexChar0)
{
    uint8_t nibble = 0x0;
    
    BYTES_EQUAL( '0', NibbleToHexChar[nibble] );
}

TEST(HexConvert, NibbleToHexCharF)
{
    uint8_t nibble = 0xF;
    
    BYTES_EQUAL( 'f', NibbleToHexChar[nibble] );
}

TEST(HexConvert, HexCharToNibble_0)
{
    __try
    {
        BYTES_EQUAL( 0, HexCharToNibble('0') );
    }
    __catch
    {
        FAIL( "HexCharToNibble threw unexpected exception." );
    }
}

TEST(HexConvert, HexCharToNibble_9)
{
    __try
    {
        BYTES_EQUAL( 9, HexCharToNibble('9') );
    }
    __catch
    {
        FAIL( "HexCharToNibble threw unexpected exception." );
    }
}

TEST(HexConvert, HexCharToNibble_a)
{
    __try
    {
        BYTES_EQUAL( 0xa, HexCharToNibble('a') );
    }
    __catch
    {
        FAIL( "HexCharToNibble threw unexpected exception." );
    }
}

TEST(HexConvert, HexCharToNibble_f)
{
    __try
    {
        BYTES_EQUAL( 0xf, HexCharToNibble('f') );
    }
    __catch
    {
        FAIL( "HexCharToNibble threw unexpected exception." );
    }
}

TEST(HexConvert, HexCharToNibble_A)
{
    __try
    {
        BYTES_EQUAL( 0xa, HexCharToNibble('A') );
    }
    __catch
    {
        FAIL( "HexCharToNibble threw unexpected exception." );
    }
}

TEST(HexConvert, HexCharToNibble_F)
{
    __try
    {
        BYTES_EQUAL( 0xf, HexCharToNibble('F') );
    }
    __catch
    {
        FAIL( "HexCharToNibble threw unexpected exception." );
    }
}

TEST(HexConvert, HexCharToNibble_InvalidG)
{
    int value;
    int exceptionThrown = 0;
    
    __try
        value = HexCharToNibble('G');
    __catch
        exceptionThrown = 1;
        
    LONGS_EQUAL( -1, value );
    CHECK_TRUE( exceptionThrown );
    LONGS_EQUAL( invalidHexDigitException, getExceptionCode() );
}

TEST(HexConvert, HexCharToNibble_Invalidg)
{
    int value;
    int exceptionThrown = 0;
    
    __try
        value = HexCharToNibble('g');
    __catch
        exceptionThrown = 1;
        
    LONGS_EQUAL( -1, value );
    CHECK_TRUE( exceptionThrown );
    LONGS_EQUAL( invalidHexDigitException, getExceptionCode() );
}
