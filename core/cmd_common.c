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
/* Common functionality shared between gdb command handlers in mri. */
#include "cmd_common.h"


void ReadAddressAndLengthArguments(Buffer* pBuffer, AddressLength* pArguments)
{
    __try
    {
        __throwing_func( pArguments->address = ReadUIntegerArgument(pBuffer) );
        __throwing_func( ThrowIfNextCharIsNotEqualTo(pBuffer, ',') );
        __throwing_func( pArguments->length = ReadUIntegerArgument(pBuffer) );
    }
    __catch
    {
        __rethrow;
    }
}


void ReadAddressAndLengthArgumentsWithColon(Buffer* pBuffer, AddressLength* pArguments)
{
    __try
    {
        __throwing_func( ReadAddressAndLengthArguments(pBuffer, pArguments) );
        __throwing_func( ThrowIfNextCharIsNotEqualTo(pBuffer, ':') );
    }
    __catch
    {
        __rethrow;
    }
}


uint32_t ReadUIntegerArgument(Buffer* pBuffer)
{
    uint32_t value;
    
    __try
        value = Buffer_ReadUIntegerAsHex(pBuffer);
    __catch
        __rethrow_and_return(0);

    return value;
}


void ThrowIfNextCharIsNotEqualTo(Buffer* pBuffer, char thisChar)
{
    if (!Buffer_IsNextCharEqualTo(pBuffer, thisChar))
        __throw(invalidArgumentException);
}
