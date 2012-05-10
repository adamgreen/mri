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
/* Handler for gdb query commands. */
#include "buffer.h"
#include "core.h"
#include "platforms.h"
#include "mri.h"
#include "cmd_common.h"
#include "cmd_query.h"


static uint32_t handleQuerySupportedCommand(void);
static uint32_t handleQueryTransferCommand(void);
static uint32_t handleQueryTransferMemoryMapCommand(void);
static void     readQueryTransferMemoryMapReadArguments(Buffer* pBuffer, AddressLength* pOffsetLength);
static void     handleMemoryMapReadCommand(AddressLength* pOffsetLength);
/* Handle the 'q' command used by gdb to communicate state to debug monitor and vice versa.

    Command Format: qSSS
    Where SSS is a variable length string indicating which query command is being sent to the stub.
*/
uint32_t HandleQueryCommand(void)
{
    Buffer*             pBuffer = GetBuffer();
    static const char   qSupportedCommand[] = "Supported";
    static const char   qXferCommand[] = "Xfer";
    
    if (Buffer_MatchesString(pBuffer, qSupportedCommand, sizeof(qSupportedCommand)-1))
    {
        return handleQuerySupportedCommand();
    }
    else if (Buffer_MatchesString(pBuffer, qXferCommand, sizeof(qXferCommand)-1))
    {
        return handleQueryTransferCommand();
    }
    else
    {
        PrepareEmptyResponseForUnknownCommand();
        return 0;
    }
}

/* Handle the "qSupported" command used by gdb to communicate state to debug monitor and vice versa.

    Reponse Format: qXfer:memory-map:read+;PacketSize==SSSSSSSS
    Where SSSSSSSS is the hexadecimal representation of the maximum packet size support by this stub.
*/
static uint32_t handleQuerySupportedCommand(void)
{
    static const char querySupportResponse[] = "qXfer:memory-map:read+;PacketSize=";
    uint32_t          PacketSize = Platform_GetPacketBufferSize();
    Buffer*           pBuffer = GetInitializedBuffer();

    Buffer_WriteString(pBuffer, querySupportResponse);
    Buffer_WriteUIntegerAsHex(pBuffer, PacketSize);
    
    return 0;
}

/* Handle the "qXfer" command used by gdb to transfer data to and from the stub for special functionality.

    Command Format: qXfer:object:read:annex:offset,length
    Where supported objects are currently:
        memory-map
*/
static uint32_t handleQueryTransferCommand(void)
{
    Buffer*             pBuffer =GetBuffer();
    static const char   memoryMapObject[] = "memory-map";
    
    if (!Buffer_IsNextCharEqualTo(pBuffer, ':'))
    {
        PrepareStringResponse(MRI_ERROR_INVALID_ARGUMENT);
        return 0;
    }
    
    if (Buffer_MatchesString(pBuffer, memoryMapObject, sizeof(memoryMapObject)-1))
    {
        return handleQueryTransferMemoryMapCommand();
    }
    else
    {
        PrepareEmptyResponseForUnknownCommand();
        return 0;
    }
}

/* Handle the "qXfer:memory-map" command used by gdb to read the device memory map from the stub.

    Command Format: qXfer:memory-map:read::offset,length
*/
static uint32_t handleQueryTransferMemoryMapCommand(void)
{
    Buffer*             pBuffer =GetBuffer();
    AddressLength       offsetLength;
    
    __try
    {
        readQueryTransferMemoryMapReadArguments(pBuffer, &offsetLength);
    }
    __catch
    {
        PrepareStringResponse(MRI_ERROR_INVALID_ARGUMENT);
        return 0;
    }

    handleMemoryMapReadCommand(&offsetLength);
    
    return 0;
}

static void readQueryTransferMemoryMapReadArguments(Buffer* pBuffer, AddressLength* pOffsetLength)
{
    static const char   ReadCommand[] = "read";

    if (!Buffer_IsNextCharEqualTo(pBuffer, ':') ||
        !Buffer_MatchesString(pBuffer, ReadCommand, sizeof(ReadCommand)-1) ||
        !Buffer_IsNextCharEqualTo(pBuffer, ':') ||
        !Buffer_IsNextCharEqualTo(pBuffer, ':'))
    {
        __throw(invalidArgumentException);
    }
    
    ReadAddressAndLengthArguments(pBuffer, pOffsetLength);
}

static void handleMemoryMapReadCommand(AddressLength* pOffsetLength)
{
    Buffer*  pBuffer = GetBuffer();
    char     dataPrefixChar = 'm';
    uint32_t offset = pOffsetLength->address;
    uint32_t length = pOffsetLength->length;
    uint32_t outputBufferSize;
    uint32_t validMemoryMapBytes;
    
    if (offset >= Platform_GetDeviceMemoryMapXmlSize())
    {
        /* Attempt to read past end of XML content so flag with a l only packet. */
        dataPrefixChar = 'l';
        length = 0;
        validMemoryMapBytes = 0;
    }
    else
    {
        validMemoryMapBytes = Platform_GetDeviceMemoryMapXmlSize() - offset;
    }
    
    InitBuffer();
    outputBufferSize = Buffer_BytesLeft(pBuffer);

    if (length > outputBufferSize)
        length = outputBufferSize;

    if (length > validMemoryMapBytes)
    {
        dataPrefixChar = 'l';
        length = validMemoryMapBytes;
    }
    
    Buffer_WriteChar(pBuffer, dataPrefixChar);
    Buffer_WriteSizedString(pBuffer, Platform_GetDeviceMemoryMapXml() + offset, length);
}
