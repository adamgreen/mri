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
/* Handler for continue gdb command. */
#include "buffer.h"
#include "core.h"
#include "platforms.h"
#include "cmd_common.h"
#include "cmd_continue.h"


static int shouldSkipHardcodedBreakpoint(void);
static int isCurrentInstructionHardcodedBreakpoint(void);
/* Handle the 'c' command which is sent from gdb to tell the debugger to continue execution of the currently halted
   program.
   
    Command Format:     cAAAAAAAA
    Response Format:    Blank until the next exception, at which time a 'T' stop response packet will be sent.

    Where AAAAAAAA is an optional value to be used for the Program Counter when restarting the program.
*/
uint32_t HandleContinueCommand(void)
{
    Buffer*     pBuffer = GetBuffer();
    uint32_t    returnValue = 0;
    uint32_t    newPC;

    if (shouldSkipHardcodedBreakpoint())
    {
        Platform_AdvanceProgramCounterToNextInstruction();
        returnValue |= HANDLER_RETURN_SKIPPED_OVER_BREAK;
    }

    /* New program counter value is optional parameter. */
    __try
    {
        __throwing_func( newPC = ReadUIntegerArgument(pBuffer) );
        Platform_SetProgramCounter(newPC);
    }
    __catch
    {
        clearExceptionCode();
    }
    
    return (returnValue | HANDLER_RETURN_RESUME_PROGRAM | HANDLER_RETURN_RETURN_IMMEDIATELY);
}

static int shouldSkipHardcodedBreakpoint(void)
{
    return !Platform_WasProgramCounterModifiedByUser() && isCurrentInstructionHardcodedBreakpoint();
}

static int isCurrentInstructionHardcodedBreakpoint(void)
{
    return Platform_TypeOfCurrentInstruction() == MRI_PLATFORM_INSTRUCTION_HARDCODED_BREAKPOINT;
}
