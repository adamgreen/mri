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
/* Semihost functionality for redirecting operations such as file I/O to the GNU debugger. */
#include <platforms.h>
#include <semihost.h>


int Semihost_IsDebuggeeMakingSemihostCall(void)
{
    PlatformInstructionType instructionType = Platform_TypeOfCurrentInstruction();

    return (instructionType == MRI_PLATFORM_INSTRUCTION_MBED_SEMIHOST_CALL ||
            instructionType == MRI_PLATFORM_INSTRUCTION_NEWLIB_SEMIHOST_CALL);
}

int Semihost_HandleSemihostRequest(void)
{
    PlatformInstructionType    instructionType = Platform_TypeOfCurrentInstruction();
    PlatformSemihostParameters parameters = Platform_GetSemihostCallParameters();

    if (instructionType == MRI_PLATFORM_INSTRUCTION_MBED_SEMIHOST_CALL)
        return Semihost_HandleMbedSemihostRequest(&parameters);
    else if (instructionType == MRI_PLATFORM_INSTRUCTION_NEWLIB_SEMIHOST_CALL)
        return Semihost_HandleNewlibSemihostRequest(&parameters);
    else
        return 0;
}
