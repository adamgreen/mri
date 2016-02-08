/* Copyright 2015 Adam Green (http://mbed.org/users/AdamGreen/)

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
/* Routines which expose Micromint Bambino210 specific functionality to the mri debugger. */
#include <string.h>
#include <platforms.h>
#include <try_catch.h>
#include "../../architectures/armv7-m/debug_cm3.h"
#include "../../devices/lpc43xx/lpc43xx_init.h"


void Platform_Init(Token* pParameterTokens)
{
    __mriLpc43xx_Init(pParameterTokens);
}


const uint8_t* __mriPlatform_GetUid(void)
{
    return NULL;
}


uint32_t __mriPlatform_GetUidSize(void)
{
    return 0;
}
