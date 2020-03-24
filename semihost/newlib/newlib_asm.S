/* Copyright 2012 Adam Green (https://github.com/adamgreen/)

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
/* Implementation of semihost assembly language routines to be used to redirect standard I/O (stdin/stdout/stderr)
   from the newlib library to the MRI debug monitor. */
    .text
    .code 16
    .syntax unified

    .global mriNewlib_SemihostWrite
    .type mriNewlib_SemihostWrite, function
    /* extern "C" int mriNewlib_SemihostWrite(int file, char *ptr, int len);
       Issues semihost write call to PC via gdb.
    */
mriNewlib_SemihostWrite:
    bkpt    0xff
    bx      lr


    .global mriNewlib_SemihostRead
    .type mriNewlib_SemihostRead, function
    /* extern "C" int mriNewlib_SemihostRead(int file, char *ptr, int len);
       Issues semihost read call to PC via gdb.
    */
mriNewlib_SemihostRead:
    bkpt    0xff
    bx      lr


    .global mriNewLib_SemihostOpen
    .type mriNewLib_SemihostOpen, function
    /* extern "C" int mriNewLib_SemihostOpen(const char *pFilename, int flags, int mode);
       Issues semihost open file call to PC via gdb.
    */
mriNewLib_SemihostOpen:
    bkpt    0xff
    bx      lr


    .global mriNewLib_SemihostRename
    .type mriNewLib_SemihostRename, function
    /* extern "C" int mriNewLib_SemihostRename(const char *pOldFilename, const char *pNewFilename);
       Issues file rename call to PC via GDB.
    */
mriNewLib_SemihostRename:
    bkpt    0xff
    bx      lr

    .global mriNewLib_SemihostUnlink
    .type mriNewLib_SemihostUnlink, function
    /* extern "C" int mriNewLib_SemihostUnlink(const char *pFilename);
       Issues file delete (unlink) call to PC via GDB.
    */
mriNewLib_SemihostUnlink:
    bkpt    0xff
    bx      lr


    .global mriNewLib_SemihostStat
    .type mriNewLib_SemihostStat, function
    /* extern "C" int mriNewLib_SemihostStat(const char *pFilename, struct stat *pStat);
       Issues stat call to PC via GDB.
    */
mriNewLib_SemihostStat:
    bkpt    0xff
    bx      lr


    .global mriNewlib_SemihostLSeek
    .type mriNewlib_SemihostLSeek, function
    /* extern "C" int mriNewlib_SemihostLSeek(int file, int offset, int whence);
       Issues seek call to PC via GDB.
    */
mriNewlib_SemihostLSeek:
    bkpt    0xff
    bx      lr


    .global mriNewlib_SemihostClose
    .type mriNewlib_SemihostClose, function
    /* extern "C" int mriNewlib_SemihostClose(int file);
       Issues file close call to PC via GDB.
    */
mriNewlib_SemihostClose:
    bkpt    0xff
    bx      lr


    .global mriNewlib_SemihostFStat
    .type mriNewlib_SemihostFStat, function
    /* extern "C" int mriNewlib_SemihostFStat(int file, struct stat *pStat);
       Issues stat call to PC via GDB.
    */
mriNewlib_SemihostFStat:
    bkpt    0xff
    bx      lr
