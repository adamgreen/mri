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
/* Semihost functionality for redirecting ARM semi-hosting operations (used by mbed LocalFileSystem) to the GNU host. */
#include <stdint.h>
#include <core/libc.h>
#include <core/core.h>
#include <core/semihost.h>
#include <core/cmd_file.h>
#include <core/fileio.h>
#include <core/mbedsys.h>


static uint32_t convertRealViewOpenModeToPosixOpenFlags(uint32_t openMode);
static int      handleArmSemihostOpenRequest(PlatformSemihostParameters* pSemihostParameters);
static int      handleArmSemihostIsTtyRequest(PlatformSemihostParameters* pSemihostParameters);
static void     convertBytesTransferredToBytesNotTransferred(int bytesThatWereToBeTransferred);
static int      handleArmSemihostWriteRequest(PlatformSemihostParameters* pSemihostParameters);
static int      handleArmSemihostCloseRequest(PlatformSemihostParameters* pSemihostParameters);
static int      handleArmSemihostReadRequest(PlatformSemihostParameters* pSemihostParameters);
static int      handleArmSemihostSeekRequest(PlatformSemihostParameters* pSemihostParameters);
static uint32_t extractWordFromBigEndianByteArray(const void* pBigEndianValueToExtract);
static int      handleArmSemihostFileLengthRequest(PlatformSemihostParameters* pSemihostParameters);
static int      handleArmSemihostRemoveRequest(PlatformSemihostParameters* pSemihostParameters);
static int      handleArmSemihostRenameRequest(PlatformSemihostParameters* pSemihostParameters);
static int      handleArmSemihostErrorNoRequest(PlatformSemihostParameters* pSemihostParameters);
int Semihost_HandleArmSemihostRequest(PlatformSemihostParameters* pParameters)
{
    uint32_t opCode;

    opCode = pParameters->parameter1;
    switch (opCode)
    {
    case 1:
        return handleArmSemihostOpenRequest(pParameters);
    case 2:
        return handleArmSemihostCloseRequest(pParameters);
    case 5:
        return handleArmSemihostWriteRequest(pParameters);
    case 6:
        return handleArmSemihostReadRequest(pParameters);
    case 9:
        return handleArmSemihostIsTtyRequest(pParameters);
    case 10:
        return handleArmSemihostSeekRequest(pParameters);
    case 12:
        return handleArmSemihostFileLengthRequest(pParameters);
    case 14:
        return handleArmSemihostRemoveRequest(pParameters);
    case 15:
        return handleArmSemihostRenameRequest(pParameters);
    case 19:
        return handleArmSemihostErrorNoRequest(pParameters);
    default:
        return 0;
    }
}

static int handleArmSemihostOpenRequest(PlatformSemihostParameters* pSemihostParameters)
{
    struct
    {
        uint32_t filenameAddress;
        uint32_t openMode;
        uint32_t filenameLength;
    } armParameters;
    uint32_t bytesRead = Platform_ReadMemory(&armParameters, pSemihostParameters->parameter2, sizeof(armParameters));
    if (bytesRead != sizeof(armParameters))
    {
        return 0;
    }

    OpenParameters parameters;
    parameters.filenameAddress = armParameters.filenameAddress;
    parameters.flags = convertRealViewOpenModeToPosixOpenFlags(armParameters.openMode);
    parameters.mode = GDB_S_IRUSR | GDB_S_IWUSR | GDB_S_IRGRP | GDB_S_IWGRP | GDB_S_IROTH | GDB_S_IWOTH;
    parameters.filenameLength = armParameters.filenameLength + 1;

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

static int handleArmSemihostIsTtyRequest(PlatformSemihostParameters* pSemihostParameters)
{
    // Just need to advance the program counter past the semi-host bkpt instruction.
    Platform_AdvanceProgramCounterToNextInstruction();
    // Just going to hardcode all such file handles to non-TTY so that they are buffered.
    Platform_SetSemihostCallReturnAndErrnoValues(0, 0);

    return 1;
}

static int handleArmSemihostWriteRequest(PlatformSemihostParameters* pSemihostParameters)
{
    TransferParameters parameters;
    uint32_t bytesRead = Platform_ReadMemory(&parameters, pSemihostParameters->parameter2, sizeof(parameters));
    if (bytesRead != sizeof(parameters))
    {
        return 0;
    }

    int returnValue = Semihost_WriteToFileOrConsole(&parameters);
    if (returnValue)
    {
        convertBytesTransferredToBytesNotTransferred(parameters.bufferSize);
    }
    return returnValue;
}

static int handleArmSemihostReadRequest(PlatformSemihostParameters* pSemihostParameters)
{
    TransferParameters parameters;
    uint32_t bytesRead = Platform_ReadMemory(&parameters, pSemihostParameters->parameter2, sizeof(parameters));
    if (bytesRead != sizeof(parameters))
    {
        return 0;
    }

    int returnValue = IssueGdbFileReadRequest(&parameters);
    if (returnValue)
    {
        convertBytesTransferredToBytesNotTransferred(parameters.bufferSize);
    }
    return returnValue;
}

static void convertBytesTransferredToBytesNotTransferred(int bytesThatWereToBeTransferred)
{
    int bytesTransferred = GetSemihostReturnCode();

    /* The ARM version of the read/write function need bytes not transferred instead of bytes transferred. */
    if (bytesTransferred >= 0)
        Platform_SetSemihostCallReturnAndErrnoValues(bytesThatWereToBeTransferred - bytesTransferred, 0);
}

static int handleArmSemihostCloseRequest(PlatformSemihostParameters* pSemihostParameters)
{
    struct
    {
        uint32_t fileDescriptor;
    } parameters;
    uint32_t bytesRead = Platform_ReadMemory(&parameters, pSemihostParameters->parameter2, sizeof(parameters));
    if (bytesRead != sizeof(parameters))
    {
        return 0;
    }

    return IssueGdbFileCloseRequest(parameters.fileDescriptor);
}

static int handleArmSemihostSeekRequest(PlatformSemihostParameters* pSemihostParameters)
{
    struct
    {
        uint32_t fileDescriptor;
        int32_t  offsetFromStart;
    } armParameters;
    uint32_t bytesRead = Platform_ReadMemory(&armParameters, pSemihostParameters->parameter2, sizeof(armParameters));
    if (bytesRead != sizeof(armParameters))
    {
        return 0;
    }

    SeekParameters parameters;
    parameters.fileDescriptor = armParameters.fileDescriptor;
    parameters.offset = armParameters.offsetFromStart;
    parameters.whence = GDB_SEEK_SET;
    return IssueGdbFileSeekRequest(&parameters);
}

static int handleArmSemihostFileLengthRequest(PlatformSemihostParameters* pSemihostParameters)
{
    struct
    {
        uint32_t fileDescriptor;
    } parameters;
    uint32_t bytesRead = Platform_ReadMemory(&parameters, pSemihostParameters->parameter2, sizeof(parameters));
    if (bytesRead != sizeof(parameters))
    {
        return 0;
    }

    GdbStats gdbFileStats;
    int returnValue = IssueGdbFileFStatRequest(parameters.fileDescriptor, (uint32_t)&gdbFileStats);
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

static int handleArmSemihostRemoveRequest(PlatformSemihostParameters* pSemihostParameters)
{
    RemoveParameters parameters;
    uint32_t bytesRead = Platform_ReadMemory(&parameters, pSemihostParameters->parameter2, sizeof(parameters));
    if (bytesRead != sizeof(parameters))
    {
        return 0;
    }

    parameters.filenameLength++;
    return IssueGdbFileUnlinkRequest(&parameters);
}

static int handleArmSemihostRenameRequest(PlatformSemihostParameters* pSemihostParameters)
{
    RenameParameters parameters;
    uint32_t bytesRead = Platform_ReadMemory(&parameters, pSemihostParameters->parameter2, sizeof(parameters));
    if (bytesRead != sizeof(parameters))
    {
        return 0;
    }

    parameters.origFilenameLength++;
    parameters.newFilenameLength++;
    return IssueGdbFileRenameRequest(&parameters);
}

static int handleArmSemihostErrorNoRequest(PlatformSemihostParameters* pSemihostParameters)
{
    Platform_AdvanceProgramCounterToNextInstruction();
    Platform_SetSemihostCallReturnAndErrnoValues(GetSemihostErrno(), 0);

    return 1;
}
