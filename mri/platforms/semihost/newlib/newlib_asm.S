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
/* Implementation of semihost assembly language routines to be used to redirect standard I/O (stdin/stdout/stderr)
   from the newlib library to the MRI debug monitor. */
    .text
    .code 16
    .syntax unified

    .global __mriNewlib_SemihostWrite
    .type __mriNewlib_SemihostWrite, function
    /* extern "C" int __mriNewlib_SemihostWrite(int file, char *ptr, int len);
       Issues semihost write call to PC via gdb.
    */
__mriNewlib_SemihostWrite:
    bkpt    0xff
    bx      lr


    .global __mriNewlib_SemihostRead
    .type __mriNewlib_SemihostRead, function
    /* extern "C" int __mriNewlib_SemihostRead(int file, char *ptr, int len);
       Issues semihost read call to PC via gdb.
    */
__mriNewlib_SemihostRead:
    bkpt    0xff
    bx      lr
