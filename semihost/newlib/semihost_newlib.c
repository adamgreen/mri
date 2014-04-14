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
/* Semihost functionality for redirecting stdin/stdout/stderr I/O to the GNU console. */
#include <string.h>
#include <mri.h>
#include <semihost.h>
#include <cmd_file.h>


static int handleNewlibSemihostWriteRequest(PlatformSemihostParameters* pSemihostParameters);
static int handleNewlibSemihostReadRequest(PlatformSemihostParameters* pSemihostParameters);
static int handleNewlibSemihostOpenRequest(PlatformSemihostParameters* pSemihostParameters);
static int handleNewlibSemihostUnlinkRequest(PlatformSemihostParameters* pSemihostParameters);
static int handleNewlibSemihostLSeekRequest(PlatformSemihostParameters* pSemihostParameters);
static int handleNewlibSemihostCloseRequest(PlatformSemihostParameters* pSemihostParameters);
static int handleNewlibSemihostFStatRequest(PlatformSemihostParameters* pSemihostParameters);
static int handleNewlibSemihostStatRequest(PlatformSemihostParameters* pSemihostParameters);
static int handleNewlibSemihostRenameRequest(PlatformSemihostParameters* pSemihostParameters);
int Semihost_HandleNewlibSemihostRequest(PlatformSemihostParameters* pSemihostParameters)
{
    uint32_t semihostOperation;
    
    semihostOperation = Platform_GetProgramCounter() | 1;
    if (semihostOperation == (uint32_t)__mriNewlib_SemihostWrite)
        return handleNewlibSemihostWriteRequest(pSemihostParameters);
    else if (semihostOperation == (uint32_t)__mriNewlib_SemihostRead)
        return handleNewlibSemihostReadRequest(pSemihostParameters);
    else if (semihostOperation == (uint32_t)__mriNewLib_SemihostOpen)
        return handleNewlibSemihostOpenRequest(pSemihostParameters);
    else if (semihostOperation == (uint32_t)__mriNewLib_SemihostUnlink)
        return handleNewlibSemihostUnlinkRequest(pSemihostParameters);
    else if (semihostOperation == (uint32_t)__mriNewlib_SemihostLSeek)
        return handleNewlibSemihostLSeekRequest(pSemihostParameters);
    else if (semihostOperation == (uint32_t)__mriNewlib_SemihostClose)
        return handleNewlibSemihostCloseRequest(pSemihostParameters);
    else if (semihostOperation == (uint32_t)__mriNewlib_SemihostFStat)
        return handleNewlibSemihostFStatRequest(pSemihostParameters);
    else if (semihostOperation == (uint32_t)__mriNewLib_SemihostStat)
        return handleNewlibSemihostStatRequest(pSemihostParameters);
    else if (semihostOperation == (uint32_t)__mriNewLib_SemihostRename)
        return handleNewlibSemihostRenameRequest(pSemihostParameters);
    else
        return 0;
}

static int handleNewlibSemihostWriteRequest(PlatformSemihostParameters* pSemihostParameters)
{
    TransferParameters parameters;

    parameters.fileDescriptor = pSemihostParameters->parameter1;
    parameters.bufferAddress = pSemihostParameters->parameter2;
    parameters.bufferSize = pSemihostParameters->parameter3;
    
    return IssueGdbFileWriteRequest(&parameters);
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

    parameters.pFilename = (const char*)pSemihostParameters->parameter1;
    parameters.filenameLength = strlen(parameters.pFilename);
    parameters.flags = pSemihostParameters->parameter2;
    parameters.mode = pSemihostParameters->parameter3;
    
    return IssueGdbFileOpenRequest(&parameters);
}

static int handleNewlibSemihostUnlinkRequest(PlatformSemihostParameters* pSemihostParameters)
{
    RemoveParameters parameters;

    parameters.filenameAddress = pSemihostParameters->parameter1;    
    parameters.filenameLength = strlen((char*)pSemihostParameters->parameter1);
    
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
    return IssueGdbFileStatRequest((const char*)pSemihostParameters->parameter1, pSemihostParameters->parameter2);
}

static int handleNewlibSemihostRenameRequest(PlatformSemihostParameters* pSemihostParameters)
{
    return IssueGdbFileRenameRequest((const char*)pSemihostParameters->parameter1, 
                                     (const char*)pSemihostParameters->parameter2);
}
