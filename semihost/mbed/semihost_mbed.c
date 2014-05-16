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
/* Semihost functionality for redirecting mbed LocalFileSystem operations to the GNU host. */
#include <stdint.h>
#include <string.h>
#include <core.h>
#include <semihost.h>
#include <cmd_file.h>
#include <fileio.h>
#include <mbedsys.h>
#include "../../boards/mbed1768/mbed1768.h"


static int      handleMbedSemihostUidRequest(PlatformSemihostParameters* pParameters);
static uint32_t convertRealViewOpenModeToPosixOpenFlags(uint32_t openMode);
static int      handleMbedSemihostOpenRequest(PlatformSemihostParameters* pSemihostParameters);
static int      handleMbedSemihostIsTtyRequest(PlatformSemihostParameters* pSemihostParameters);
static void     convertBytesTransferredToBytesNotTransferred(int bytesThatWereToBeTransferred);
static int      handleMbedSemihostWriteRequest(PlatformSemihostParameters* pSemihostParameters);
static int      handleMbedSemihostCloseRequest(PlatformSemihostParameters* pSemihostParameters);
static int      handleMbedSemihostReadRequest(PlatformSemihostParameters* pSemihostParameters);
static int      handleMbedSemihostSeekRequest(PlatformSemihostParameters* pSemihostParameters);
static uint32_t extractWordFromBigEndianByteArray(const void* pBigEndianValueToExtract);
static int      handleMbedSemihostFileLengthRequest(PlatformSemihostParameters* pSemihostParameters);
static int      handleMbedSemihostRemoveRequest(PlatformSemihostParameters* pSemihostParameters);
int Semihost_HandleMbedSemihostRequest(PlatformSemihostParameters* pParameters)
{
    uint32_t opCode;
    
    opCode = pParameters->parameter1;
    switch (opCode)
    {
    case 1:
        return handleMbedSemihostOpenRequest(pParameters);
    case 2:
        return handleMbedSemihostCloseRequest(pParameters);
    case 5:
        return handleMbedSemihostWriteRequest(pParameters);
    case 6:
        return handleMbedSemihostReadRequest(pParameters);
    case 9:
        return handleMbedSemihostIsTtyRequest(pParameters);
    case 10:
        return handleMbedSemihostSeekRequest(pParameters);
    case 12:
        return handleMbedSemihostFileLengthRequest(pParameters);
    case 14:
        return handleMbedSemihostRemoveRequest(pParameters);
    case 257:
        return handleMbedSemihostUidRequest(pParameters);
    default:
        return 0;
    }
}

static int handleMbedSemihostUidRequest(PlatformSemihostParameters* pParameters)
{
    typedef struct
    {
        uint8_t*   pBuffer;
        uint32_t   bufferSize;
    } SUidParameters;
    const SUidParameters* pUidParameters;
    uint32_t              copySize;
    
    pUidParameters = (const SUidParameters*)pParameters->parameter2;
    copySize = pUidParameters->bufferSize;
    if (copySize > MBED1768_UID_SIZE)
        copySize = MBED1768_UID_SIZE;
    memcpy(pUidParameters->pBuffer, __mriMbed1768_GetMbedUid(), copySize);

    Platform_AdvanceProgramCounterToNextInstruction();
    Platform_SetSemihostCallReturnAndErrnoValues(0, 0);

    return 1;
}

static int handleMbedSemihostOpenRequest(PlatformSemihostParameters* pSemihostParameters)
{
    typedef struct
    {
        uint32_t filenameAddress;
        uint32_t openMode;
        uint32_t filenameLength;    
    } MbedOpenParameters;
    const MbedOpenParameters*  pParameters = (const MbedOpenParameters*)pSemihostParameters->parameter2;
    OpenParameters             parameters;
    
    parameters.filenameAddress = pParameters->filenameAddress;
    parameters.flags = convertRealViewOpenModeToPosixOpenFlags(pParameters->openMode);
    parameters.mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
    parameters.filenameLength = pParameters->filenameLength + 1;
    
    return IssueGdbFileOpenRequest(&parameters); 
}

static uint32_t convertRealViewOpenModeToPosixOpenFlags(uint32_t openMode)
{
    uint32_t posixOpenMode = 0;
    uint32_t posixOpenDisposition = 0;

    if (openMode & OPENMODE_W)
    {
        posixOpenMode = O_WRONLY;
        posixOpenDisposition = O_CREAT | O_TRUNC;
    }
    else if (openMode & OPENMODE_A)
    {
        posixOpenMode = O_WRONLY ;
        posixOpenDisposition = O_CREAT | O_APPEND;
    }
    else
    {
        posixOpenMode = O_RDONLY;
        posixOpenDisposition = 0;
    }
    if (openMode & OPENMODE_PLUS)
    {
        posixOpenMode = O_RDWR;
    }
    
    return posixOpenMode | posixOpenDisposition;
}

static int handleMbedSemihostIsTtyRequest(PlatformSemihostParameters* pSemihostParameters)
{
    typedef struct
    {
        uint32_t        fileDescriptor;
    } IsTtyParameters;
    const IsTtyParameters* pParameters;
    
    pParameters = (const IsTtyParameters*)pSemihostParameters->parameter2;
    (void)pParameters;

    Platform_AdvanceProgramCounterToNextInstruction();

    // Hardcode all such file handles to non-TTY so that they are buffered.
    Platform_SetSemihostCallReturnAndErrnoValues(0, 0);

    return 1;
}

static int handleMbedSemihostWriteRequest(PlatformSemihostParameters* pSemihostParameters)
{
    const TransferParameters*  pParameters = (const TransferParameters*)pSemihostParameters->parameter2;
    int returnValue;

    returnValue = IssueGdbFileWriteRequest(pParameters); 
    if (returnValue)
        convertBytesTransferredToBytesNotTransferred(pParameters->bufferSize);
    
    return returnValue;
}

static int handleMbedSemihostReadRequest(PlatformSemihostParameters* pSemihostParameters)
{
    const TransferParameters*  pParameters = (const TransferParameters*)pSemihostParameters->parameter2;
    int   returnValue;
    
    returnValue = IssueGdbFileReadRequest(pParameters); 
    if (returnValue)
        convertBytesTransferredToBytesNotTransferred(pParameters->bufferSize);
    
    return returnValue;
}

static void convertBytesTransferredToBytesNotTransferred(int bytesThatWereToBeTransferred)
{
    int bytesTransferred = GetSemihostReturnCode();
    
    /* The mbed version of the read/write function need bytes not transferred instead of bytes transferred. */
    if (bytesTransferred >= 0)
        Platform_SetSemihostCallReturnAndErrnoValues(bytesThatWereToBeTransferred - bytesTransferred, 0);
    else
        /* Maintain error code. */
        Platform_SetSemihostCallReturnAndErrnoValues(bytesTransferred, 0);
}

static int handleMbedSemihostCloseRequest(PlatformSemihostParameters* pSemihostParameters)
{
    typedef struct
    {
        uint32_t        fileDescriptor;
    } CloseParameters;
    const CloseParameters* pParameters = (const CloseParameters*)pSemihostParameters->parameter2;
    
    return IssueGdbFileCloseRequest(pParameters->fileDescriptor);
}

static int handleMbedSemihostSeekRequest(PlatformSemihostParameters* pSemihostParameters)
{
    typedef struct
    {
        uint32_t    fileDescriptor;
        int32_t     offsetFromStart;
    } MbedSeekParameters;
    const MbedSeekParameters* pParameters = (const MbedSeekParameters*)pSemihostParameters->parameter2;
    SeekParameters parameters;
    
    parameters.fileDescriptor = pParameters->fileDescriptor;
    parameters.offset = pParameters->offsetFromStart;
    parameters.whence = SEEK_SET;
    return IssueGdbFileSeekRequest(&parameters);
}

static int handleMbedSemihostFileLengthRequest(PlatformSemihostParameters* pSemihostParameters)
{
    typedef struct
    {
        uint32_t        fileDescriptor;
    } FileLengthParameters;
    typedef struct
    {
        uint32_t    device;
        uint32_t    inode;
        uint32_t    node;
        uint32_t    numberOfLinks;
        uint32_t    userId;
        uint32_t    groupId;
        uint32_t    deviceType;
        uint32_t    totalSizeUpperWord;
        uint32_t    totalSizeLowerWord;
        uint32_t    blockSizeUpperWord;
        uint32_t    blockSizeLowerWord;
        uint32_t    blockCountUpperWord;
        uint32_t    blockCountLowerWord;
        uint32_t    lastAccessTime;
        uint32_t    lastModifiedTime;
        uint32_t    lastChangeTime;
    } GdbStats;
    const FileLengthParameters*  pParameters = (const FileLengthParameters*)pSemihostParameters->parameter2;
    GdbStats                     gdbFileStats;
    int                          returnValue;
    
    returnValue = IssueGdbFileFStatRequest(pParameters->fileDescriptor, (uint32_t)&gdbFileStats);
    if (returnValue && GetSemihostReturnCode() == 0)
    {
        /* The stat command was successfully executed to set R0 to the file length field. */
        Platform_SetSemihostCallReturnAndErrnoValues(extractWordFromBigEndianByteArray(&gdbFileStats.totalSizeLowerWord), 0);
    }

    return returnValue;
}

static uint32_t extractWordFromBigEndianByteArray(const void* pBigEndianValueToExtract)
{
    const unsigned char* pBigEndianValue = (const unsigned char*)pBigEndianValueToExtract;
    return pBigEndianValue[3]        | (pBigEndianValue[2] << 8) |
          (pBigEndianValue[1] << 16) | (pBigEndianValue[0] << 24);
}

static int handleMbedSemihostRemoveRequest(PlatformSemihostParameters* pSemihostParameters)
{
    RemoveParameters*  pParameters = (RemoveParameters*)pSemihostParameters->parameter2;
    pParameters->filenameLength++;
    return IssueGdbFileUnlinkRequest(pParameters);
}
