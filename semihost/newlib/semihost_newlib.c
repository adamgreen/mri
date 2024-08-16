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
/* Semihost functionality for redirecting stdin/stdout/stderr I/O to the GNU console. */
#include <core/libc.h>
#include <core/mri.h>
#include <core/semihost.h>
#include <core/cmd_file.h>
#include <core/core.h>
#include "newlib_stubs.h"


static int handleNewlibSemihostWriteRequest(PlatformSemihostParameters* pSemihostParameters);
static int handleNewlibSemihostReadRequest(PlatformSemihostParameters* pSemihostParameters);
static int handleNewlibSemihostOpenRequest(PlatformSemihostParameters* pSemihostParameters);
static int handleNewlibSemihostUnlinkRequest(PlatformSemihostParameters* pSemihostParameters);
static int handleNewlibSemihostLSeekRequest(PlatformSemihostParameters* pSemihostParameters);
static int handleNewlibSemihostCloseRequest(PlatformSemihostParameters* pSemihostParameters);
static int handleNewlibSemihostFStatRequest(PlatformSemihostParameters* pSemihostParameters);
static int handleNewlibSemihostStatRequest(PlatformSemihostParameters* pSemihostParameters);
static int handleNewlibSemihostRenameRequest(PlatformSemihostParameters* pSemihostParameters);
static int handleNewlibSemihostGetErrNoRequest(PlatformSemihostParameters* pSemihostParameters);
static int handleNewlibSemihostSetHooksRequest(PlatformSemihostParameters* pSemihostParameters);
int Semihost_HandleNewlibSemihostRequest(PlatformSemihostParameters* pSemihostParameters)
{
    uintmri_t semihostOperation = Platform_GetNewlibSemihostOperation();

    switch (semihostOperation)
    {
        case MRI_NEWLIB_SEMIHOST_WRITE:
            return handleNewlibSemihostWriteRequest(pSemihostParameters);
        case MRI_NEWLIB_SEMIHOST_READ:
            return handleNewlibSemihostReadRequest(pSemihostParameters);
        case MRI_NEWLIB_SEMIHOST_OPEN:
            return handleNewlibSemihostOpenRequest(pSemihostParameters);
        case MRI_NEWLIB_SEMIHOST_UNLINK:
            return handleNewlibSemihostUnlinkRequest(pSemihostParameters);
        case MRI_NEWLIB_SEMIHOST_LSEEK:
            return handleNewlibSemihostLSeekRequest(pSemihostParameters);
        case MRI_NEWLIB_SEMIHOST_CLOSE:
            return handleNewlibSemihostCloseRequest(pSemihostParameters);
        case MRI_NEWLIB_SEMIHOST_FSTAT:
            return handleNewlibSemihostFStatRequest(pSemihostParameters);
        case MRI_NEWLIB_SEMIHOST_STAT:
            return handleNewlibSemihostStatRequest(pSemihostParameters);
        case MRI_NEWLIB_SEMIHOST_RENAME:
            return handleNewlibSemihostRenameRequest(pSemihostParameters);
        case MRI_NEWLIB_SEMIHOST_GET_ERRNO:
            return handleNewlibSemihostGetErrNoRequest(pSemihostParameters);
        case MRI_NEWLIB_SEMIHOST_SET_HOOKS:
            return handleNewlibSemihostSetHooksRequest(pSemihostParameters);
        default:
            return 0;
    }
}

static int handleNewlibSemihostWriteRequest(PlatformSemihostParameters* pSemihostParameters)
{
    TransferParameters parameters;

    parameters.fileDescriptor = pSemihostParameters->parameter1;
    parameters.bufferAddress = pSemihostParameters->parameter2;
    parameters.bufferSize = pSemihostParameters->parameter3;

    return Semihost_WriteToFileOrConsole(&parameters);
}

static int handleNewlibSemihostReadRequest(PlatformSemihostParameters* pSemihostParameters)
{
    TransferParameters parameters;

    parameters.fileDescriptor = pSemihostParameters->parameter1;
    parameters.bufferAddress = pSemihostParameters->parameter2;
    parameters.bufferSize = pSemihostParameters->parameter3;

    return IssueGdbFileReadRequest(&parameters);
}

static int handleNewlibSemihostOpenRequest(PlatformSemihostParameters* pSemihostParameters)
{
    OpenParameters parameters;

    parameters.filenameAddress = pSemihostParameters->parameter1;
    parameters.filenameLength = pSemihostParameters->parameter2;
    parameters.flags = pSemihostParameters->parameter3;
    parameters.mode = pSemihostParameters->parameter4;

    return IssueGdbFileOpenRequest(&parameters);
}

static int handleNewlibSemihostUnlinkRequest(PlatformSemihostParameters* pSemihostParameters)
{
    RemoveParameters parameters;

    parameters.filenameAddress = pSemihostParameters->parameter1;
    parameters.filenameLength = pSemihostParameters->parameter2;

    return IssueGdbFileUnlinkRequest(&parameters);
}

static int handleNewlibSemihostLSeekRequest(PlatformSemihostParameters* pSemihostParameters)
{
    SeekParameters parameters;

    parameters.fileDescriptor = pSemihostParameters->parameter1;
    parameters.offset = pSemihostParameters->parameter2;
    parameters.whence = pSemihostParameters->parameter3;

    return IssueGdbFileSeekRequest(&parameters);
}

static int handleNewlibSemihostCloseRequest(PlatformSemihostParameters* pSemihostParameters)
{
    return IssueGdbFileCloseRequest(pSemihostParameters->parameter1);
}

static int handleNewlibSemihostFStatRequest(PlatformSemihostParameters* pSemihostParameters)
{
    return IssueGdbFileFStatRequest(pSemihostParameters->parameter1, pSemihostParameters->parameter2);
}

static int handleNewlibSemihostStatRequest(PlatformSemihostParameters* pSemihostParameters)
{
    StatParameters parameters;

    parameters.filenameAddress = pSemihostParameters->parameter1;
    parameters.filenameLength = pSemihostParameters->parameter2;
    parameters.fileStatBuffer = pSemihostParameters->parameter3;
    return IssueGdbFileStatRequest(&parameters);
}

static int handleNewlibSemihostRenameRequest(PlatformSemihostParameters* pSemihostParameters)
{
    RenameParameters parameters;

    parameters.origFilenameAddress = pSemihostParameters->parameter1;
    parameters.origFilenameLength = pSemihostParameters->parameter2;
    parameters.newFilenameAddress = pSemihostParameters->parameter3;
    parameters.newFilenameLength = pSemihostParameters->parameter4;
    return IssueGdbFileRenameRequest(&parameters);
}

static int handleNewlibSemihostGetErrNoRequest(PlatformSemihostParameters* pSemihostParameters)
{
    SetSemihostReturnValues(GetSemihostErrno(), 0);
    FlagSemihostCallAsHandled();
    return 1;
}

static int handleNewlibSemihostSetHooksRequest(PlatformSemihostParameters* pSemihostParameters)
{
    MriDebuggerHookPtr pEnteringHook = (MriDebuggerHookPtr)pSemihostParameters->parameter1;
    MriDebuggerHookPtr pLeavingHook = (MriDebuggerHookPtr)pSemihostParameters->parameter2;
    void* pvContext = (void*)pSemihostParameters->parameter3;

    mriCoreSetDebuggerHooks(pEnteringHook, pLeavingHook, pvContext);

    SetSemihostReturnValues(0, 0);
    FlagSemihostCallAsHandled();
    return 1;
}
