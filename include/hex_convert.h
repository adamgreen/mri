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
/* Hexadecimal to/from text conversion helpers. */
#ifndef _HEX_CONVERT_H_
#define _HEX_CONVERT_H_

#include "try_catch.h"

#define EXTRACT_HI_NIBBLE(X) (((X) >> 4) & 0xF)
#define EXTRACT_LO_NIBBLE(X) ((X) & 0xF)

static const char NibbleToHexChar[16] = { '0', '1', '2', '3', '4', '5', '6', '7',
                                          '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };

static inline int HexCharToNibble(unsigned char HexChar)
{
    if (HexChar >= 'a' && HexChar <= 'f')
    {
        return HexChar - 'a' + 10;
    }
    if (HexChar >= 'A' && HexChar <= 'F')
    {
        return HexChar - 'A' + 10;
    }
    if (HexChar >= '0' && HexChar <= '9')
    {
        return HexChar - '0';
    }
    
    __throw_and_return(invalidHexDigitException, -1);
}

#endif /* _HEX_CONVERT_H_ */
