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
/* Assembly Language routines which expose mbed1768 board semi-host functionality to the mri debugger. It is based on
   the code contained in the mbed SDK for exposing its semi-host functionality.  Having these functions in this 
   module of the MRI library allows the debugger to work on mbed devices even if the application doesn't utilize the
   mbed SDK itself. */
    .text
    .code 16
    .syntax unified
    
    .global __mriDisableMbed
    .thumb_func
    /* extern "C" int __mriDisableMbed(void);
       Attempts to disable the debug interface on the mbed device.
    
       Returns -1 on failure.
    */
__mriDisableMbed:
    sub     sp, #8          // Allocate 4 byte parameter block on stack (round up to maintain 8 byte alignment.)
    add     r1, sp, #4      // R1 = pointer to 4 byte parameter block on stack.
    movw    r0, #261        // R0 = semi-host id for disabling JTAG port use from mbed interface chip.
    bkpt	0xab            // Semi-host breakpoint.
    cmp     r0, #0          // Was the call successful (returned 0 in R0)?
    ite     eq              
    ldreq   r0, [sp, #4]    //  If so load R0 with the first word from parameter block.
    movne.w r0, #-1         //  If not then set R0 to -1.
    add     sp, #8          // Restore stack pointer
    bx      lr              //  and return to caller.


    .global __mriGetMbedUid
    .thumb_func
    /* extern "C" int __mriGetMbedUid(uint8_t* pOutputBuffer);
       Reads the unique device id from the mbed interface chip.
       
       Returns -1 on failure.
    */
__mriGetMbedUid:
    sub     sp, #8          // Allocate 8 byte parameter block on stack.
    str     r0, [sp, #0]    // param[0] = pOutputBuffer  Pointer to output buffer for UID.
    movs    r0, #33
    str     r0, [sp, #4]    // param[1] = 33  Size of UID output buffer in bytes.
    mov     r1, sp          // R1 = pointer to 8 byte parameter block on stack.
    movw    r0, #257        // R0 = semi-host id for fetching mbed UID.
    bkpt    0xab            // Semi-host breakpoint.
    add     sp, #8          // Restore stack pointer
    bx      lr              //  and return to caller.
