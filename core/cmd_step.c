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
/* Handler for single step gdb command. */
#include "platforms.h"
#include "cmd_common.h"
#include "cmd_continue.h"
#include "cmd_registers.h"
#include "cmd_step.h"


static uint32_t justAdvancedPastBreakpoint(uint32_t continueReturn);
/* Handle the 's' command which is sent from gdb to tell the debugger to single step over the next instruction in the
   currently halted program.
   
    Command Format:     sAAAAAAAA
    Response Format:    Blank until the next exception, at which time a 'T' stop response packet will be sent.

    Where AAAAAAAA is an optional value to be used for the Program Counter when restarting the program.
*/
uint32_t HandleSingleStepCommand(void)
{
    /* Single step is pretty much like continue except processor is told to only execute 1 instruction. */
    if (justAdvancedPastBreakpoint(HandleContinueCommand()))
    {
        /* Treat the advance as the single step and don't resume execution. */
        return Send_T_StopResponse();
    }

    Platform_EnableSingleStep();

    return (HANDLER_RETURN_RESUME_PROGRAM | HANDLER_RETURN_RETURN_IMMEDIATELY);
}

static uint32_t justAdvancedPastBreakpoint(uint32_t continueReturn)
{
    return continueReturn & HANDLER_RETURN_SKIPPED_OVER_BREAK;
}
