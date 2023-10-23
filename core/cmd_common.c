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
/* Common functionality shared between gdb command handlers in mri. */
#include <core/cmd_common.h>


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


uintmri_t ReadUIntegerArgument(Buffer* pBuffer)
{
    uintmri_t value;

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
