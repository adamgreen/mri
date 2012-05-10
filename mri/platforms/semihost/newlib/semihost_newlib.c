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
#include <mri.h>
#include <semihost.h>
#include <cmd_file.h>


static int handleNewlibSemihostWriteRequest(PlatformSemihostParameters* pSemihostParameters);
static int handleNewlibSemihostReadRequest(PlatformSemihostParameters* pSemihostParameters);
int Semihost_HandleNewlibSemihostRequest(PlatformSemihostParameters* pSemihostParameters)
{
    uint32_t semihostOperation;
    
    semihostOperation = Platform_GetProgramCounter() | 1;
    if (semihostOperation == (uint32_t)__mriNewlib_SemihostWrite)
        return handleNewlibSemihostWriteRequest(pSemihostParameters);
    else if (semihostOperation == (uint32_t)__mriNewlib_SemihostRead)
        return handleNewlibSemihostReadRequest(pSemihostParameters);
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
