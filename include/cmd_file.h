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
/* Handling and issuing routines for gdb file commands. */
#ifndef _CMD_FILE_H_
#define _CMD_FILE_H_

#include <stdint.h>
#include "buffer.h"

typedef struct
{
    const char*     pFilename;
    uint32_t        filenameLength;    
    uint32_t        flags;
    uint32_t        mode;
} OpenParameters;

typedef struct 
{
    uint32_t        fileDescriptor;
    uint32_t        bufferAddress;
    int32_t         bufferSize;    
} TransferParameters;

typedef struct
{
    uint32_t        fileDescriptor;
    int32_t         offset;
    int32_t         whence;
} SeekParameters;

typedef struct
{
    uint32_t        filenameAddress;
    uint32_t        filenameLength;    
} RemoveParameters;

/* Real name of functions are in __mri namespace. */
int      __mriIssueGdbFileOpenRequest(const OpenParameters* pParameters);
int      __mriIssueGdbFileWriteRequest(const TransferParameters* pParameters);
int      __mriIssueGdbFileReadRequest(const TransferParameters* pParameters);
int      __mriIssueGdbFileCloseRequest(uint32_t fileDescriptor);
int      __mriIssueGdbFileSeekRequest(const SeekParameters* pParameters);
int      __mriIssueGdbFileFStatRequest(uint32_t fileDescriptor, uint32_t fileStatBuffer);
int      __mriIssueGdbFileUnlinkRequest(const RemoveParameters* pParameters);
int      __mriIssueGdbFileStatRequest(const char* pFilename, uint32_t fileStatBuffer);
int      __mriIssueGdbFileRenameRequest(const char* pOrigFilename, const char* pNewFilename);
uint32_t __mriHandleFileIOCommand(void);

/* Macroes which allow code to drop the __mri namespace prefix. */
#define IssueGdbFileOpenRequest     __mriIssueGdbFileOpenRequest
#define IssueGdbFileWriteRequest    __mriIssueGdbFileWriteRequest
#define IssueGdbFileReadRequest     __mriIssueGdbFileReadRequest
#define IssueGdbFileCloseRequest    __mriIssueGdbFileCloseRequest
#define IssueGdbFileSeekRequest     __mriIssueGdbFileSeekRequest
#define IssueGdbFileFStatRequest    __mriIssueGdbFileFStatRequest
#define IssueGdbFileUnlinkRequest   __mriIssueGdbFileUnlinkRequest
#define IssueGdbFileStatRequest     __mriIssueGdbFileStatRequest
#define IssueGdbFileRenameRequest   __mriIssueGdbFileRenameRequest
#define HandleFileIOCommand         __mriHandleFileIOCommand

#endif /* _CMD_FILE_H_ */
