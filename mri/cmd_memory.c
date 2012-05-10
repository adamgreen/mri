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
/* Handlers for memory related gdb commands. */
#include "buffer.h"
#include "core.h"
#include "mri.h"
#include "memory.h"
#include "cmd_common.h"
#include "cmd_memory.h"


/* Handle the 'm' command which is to read the specified address range from memory.

    Command Format:     mAAAAAAAA,LLLLLLLL
    Response Format:    xx...
    
    Where AAAAAAAA is the hexadecimal representation of the address where the read is to start.
          LLLLLLLL is the hexadecimal representation of the length (in bytes) of the read to be conducted.
          xx is the hexadecimal representation of the first byte read from the specified location.
          ... continue returning the rest of LLLLLLLL-1 bytes in hexadecimal format.
*/
uint32_t HandleMemoryReadCommand(void)
{
    Buffer*       pBuffer = GetBuffer();
    AddressLength addressLength;

    __try
    {
        ReadAddressAndLengthArguments(pBuffer, &addressLength);
    }
    __catch
    {
        PrepareStringResponse(MRI_ERROR_INVALID_ARGUMENT);
        return 0;
    }
        
    InitBuffer();
    ReadMemoryIntoHexBuffer(pBuffer, (unsigned char *)addressLength.address, addressLength.length);

    return 0;
}


/* Handle the 'M' command which is to write to the specified address range in memory.

    Command Format:     MAAAAAAAA,LLLLLLLL:xx...
    Response Format:    OK
    
    Where AAAAAAAA is the hexadecimal representation of the address where the write is to start.
          LLLLLLLL is the hexadecimal representation of the length (in bytes) of the write to be conducted.
          xx is the hexadecimal representation of the first byte to be written to the specified location.
          ... continue returning the rest of LLLLLLLL-1 bytes in hexadecimal format.
*/
uint32_t HandleMemoryWriteCommand(void)
{
    Buffer*         pBuffer = GetBuffer();
    AddressLength   addressLength;

    __try
    {
        ReadAddressAndLengthArgumentsWithColon(pBuffer, &addressLength);
    }
    __catch
    {
        PrepareStringResponse(MRI_ERROR_INVALID_ARGUMENT);
        return 0;
    }
    
    if (WriteHexBufferToMemory(pBuffer, (unsigned char *)addressLength.address, addressLength.length))
        PrepareStringResponse("OK");
    else
        PrepareStringResponse(MRI_ERROR_MEMORY_ACCESS_FAILURE);

    return 0;
}


static void readBinaryMemoryWriteArguments(Buffer* pBuffer, AddressLength* pAddressLength);
/* Handle the 'X' command which is to write to the specified address range in memory.

    Command Format:     XAAAAAAAA,LLLLLLLL:xx...
    Response Format:    OK
    
    Where AAAAAAAA is the hexadecimal representation of the address where the write is to start.
          LLLLLLLL is the hexadecimal representation of the length (in bytes) of the write to be conducted.
          xx is the hexadecimal representation of the first byte to be written to the specified location.
          ... continue returning the rest of LLLLLLLL-1 bytes in hexadecimal format.
*/
uint32_t HandleBinaryMemoryWriteCommand(void)
{
    Buffer*        pBuffer = GetBuffer();
    AddressLength  addressLength;

    __try
    {
        readBinaryMemoryWriteArguments(pBuffer, &addressLength);
    }
    __catch
    {
        PrepareStringResponse(MRI_ERROR_INVALID_ARGUMENT);
        return 0;
    }
    
    if (WriteBinaryBufferToMemory(pBuffer, (unsigned char *)addressLength.address, addressLength.length))
    {
        PrepareStringResponse("OK");
    }
    else
    {
        if (Buffer_OverrunDetected(pBuffer))
            PrepareStringResponse( MRI_ERROR_BUFFER_OVERRUN);
        else
            PrepareStringResponse(MRI_ERROR_MEMORY_ACCESS_FAILURE);
    }

    return 0;
}

static void readBinaryMemoryWriteArguments(Buffer* pBuffer, AddressLength* pAddressLength)
{
    __try
    {
        __throwing_func( ReadAddressAndLengthArguments(pBuffer, pAddressLength) );
        __throwing_func( ThrowIfNextCharIsNotEqualTo(pBuffer, ':') );
    }
    __catch
    {
        __rethrow;
    }
}
