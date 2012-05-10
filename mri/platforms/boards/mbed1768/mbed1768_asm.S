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
/* Assembly Language routines which expose mbed1768 board specifc functionality to the mri debugger. */
    .text
    .code 16
    .syntax unified
    
    .global __mriDisableMbed
    /* extern "C" int __mriDisableMbed(void);
       Attempts to disable the debug interface on the mbed device.
    
       Returns -1 on failure and 0 otherwise.
    */
__mriDisableMbed:
    sub     sp, #8
    add     r1, sp, #4
    movw    r0, #261
    bkpt	0x00ab
    cmp     r0, #0
    ite     eq
    ldreq   r0, [sp, #4]
    movne.w r0, #-1
    add     sp, #8
    bx      lr
