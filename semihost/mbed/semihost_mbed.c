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
/* Semihost functionality for redirecting mbed LocalFileSystem operations to the GNU host. */
#include <stdint.h>
#include <core/libc.h>
#include <core/core.h>
#include <core/semihost.h>
#include <core/cmd_file.h>
#include <core/fileio.h>
#include <core/mbedsys.h>


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
static int      handleMbedSemihostRenameRequest(PlatformSemihostParameters* pSemihostParameters);
static int      handleMbedSemihostErrorNoRequest(PlatformSemihostParameters* pSemihostParameters);
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
    case 15:
        return handleMbedSemihostRenameRequest(pParameters);
    case 19:
        return handleMbedSemihostErrorNoRequest(pParameters);
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
    uint32_t              uidSize = Platform_GetUidSize();
    const SUidParameters* pUidParameters;
    uint32_t              copySize;

    pUidParameters = (const SUidParameters*)pParameters->parameter2;
    copySize = pUidParameters->bufferSize;
    if (copySize > uidSize)
        copySize = uidSize;
    mri_memcpy(pUidParameters->pBuffer, Platform_GetUid(), copySize);

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
    parameters.mode = GDB_S_IRUSR | GDB_S_IWUSR | GDB_S_IRGRP | GDB_S_IWGRP | GDB_S_IROTH | GDB_S_IWOTH;
    parameters.filenameLength = pParameters->filenameLength + 1;

    return IssueGdbFileOpenRequest(&parameters);
}

static uint32_t convertRealViewOpenModeToPosixOpenFlags(uint32_t openMode)
{
    uint32_t posixOpenMode = 0;
    uint32_t posixOpenDisposition = 0;

    if (openMode & OPENMODE_W)
    {
        posixOpenMode = GDB_O_WRONLY;
        posixOpenDisposition = GDB_O_CREAT | GDB_O_TRUNC;
    }
    else if (openMode & OPENMODE_A)
    {
        posixOpenMode = GDB_O_WRONLY ;
        posixOpenDisposition = GDB_O_CREAT | GDB_O_APPEND;
    }
    else
    {
        posixOpenMode = GDB_O_RDONLY;
        posixOpenDisposition = 0;
    }
    if (openMode & OPENMODE_PLUS)
    {
        posixOpenMode = GDB_O_RDWR;
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

    returnValue = Semihost_WriteToFileOrConsole(pParameters);
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
    parameters.whence = GDB_SEEK_SET;
    return IssueGdbFileSeekRequest(&parameters);
}

static int handleMbedSemihostFileLengthRequest(PlatformSemihostParameters* pSemihostParameters)
{
    typedef struct
    {
        uint32_t        fileDescriptor;
    } FileLengthParameters;
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

static int handleMbedSemihostRenameRequest(PlatformSemihostParameters* pSemihostParameters)
{
    RenameParameters* pParameters = (RenameParameters*)pSemihostParameters->parameter2;
    pParameters->origFilenameLength++;
    pParameters->newFilenameLength++;
    return IssueGdbFileRenameRequest(pParameters);
}

static int handleMbedSemihostErrorNoRequest(PlatformSemihostParameters* pSemihostParameters)
{
    Platform_AdvanceProgramCounterToNextInstruction();
    Platform_SetSemihostCallReturnAndErrnoValues(GetSemihostErrno(), 0);

    return 1;
}
