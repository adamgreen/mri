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
/* Implementation of Assembly Language routines to be used by MRI debug monitor. */
    .text
    .code 16
    .syntax unified

    .pushsection .text.__mriTestRegisters, "ax" @progbits
    .global __mriTestRegisters
    .thumb_func
    /* Test out context saving and restoring. */
__mriTestRegisters:
    push {r4-r11,lr}
    mov r0, #0
    mov r1, #1
    mov r2, #2
    mov r3, #3
    mov r4, #4
    mov r5, #5
    mov r6, #6
    mov r7, #7
    mov r8, #8
    mov r9, #9
    mov r10, #10
    mov r11, #11
    mov r12, #12
    bkpt #0
    push {r12}  /* Try 8 byte aligning the stack. */
    bkpt #0
    pop {r12}
    pop {r4-r11,pc}
    .popsection

    .pushsection .text.__mriTestStackError, "ax" @progbits
    .global __mriTestStackError
    .thumb_func
    /* Test getting a stacking error when CTRL+C is pressed during this loop. */
__mriTestStackError:
    mvn     r0, #0
    msr     msp, r0
__LoopUntilBreak:
    b       __LoopUntilBreak
    .popsection
    