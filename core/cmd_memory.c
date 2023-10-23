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
/* Handlers for memory related gdb commands. */
#include <core/platforms.h>
#include <core/buffer.h>
#include <core/core.h>
#include <core/mri.h>
#include <core/memory.h>
#include <core/cmd_common.h>
#include <core/cmd_memory.h>


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
    uint32_t      result;

    __try
    {
        ReadAddressAndLengthArguments(pBuffer, &addressLength);
    }
    __catch
    {
        PrepareStringResponse(MRI_ERROR_INVALID_ARGUMENT);
        return 0;
    }

    InitPacketBuffers();
    result = ReadMemoryIntoHexBuffer(pBuffer, addressLength.address, addressLength.length);
    if (result == 0)
        PrepareStringResponse(MRI_ERROR_MEMORY_ACCESS_FAILURE);

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

    if (WriteHexBufferToMemory(pBuffer, addressLength.address, addressLength.length))
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
    uintmri_t      address;
    uintmri_t      length;

    __try
    {
        ReadAddressAndLengthArgumentsWithColon(pBuffer, &addressLength);
    }
    __catch
    {
        PrepareStringResponse(MRI_ERROR_INVALID_ARGUMENT);
        return 0;
    }

    address = addressLength.address;
    length = addressLength.length;
    if (WriteBinaryBufferToMemory(pBuffer, address, length))
    {
        Platform_SyncICacheToDCache(address, length);
        PrepareStringResponse("OK");
    }
    else
    {
        if (Buffer_OverrunDetected(pBuffer))
            PrepareStringResponse(MRI_ERROR_BUFFER_OVERRUN);
        else
            PrepareStringResponse(MRI_ERROR_MEMORY_ACCESS_FAILURE);
    }

    return 0;
}
