/* Copyright 2014 Adam Green (http://mbed.org/users/AdamGreen/)

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
#ifndef _SEMIHOST_H_
#define _SEMIHOST_H_

#include "platforms.h"

/* Real name of functions are in __mri namespace. */
int __mriSemihost_IsDebuggeeMakingSemihostCall(void);
int __mriSemihost_HandleSemihostRequest(void);
int __mriSemihost_HandleNewlibSemihostRequest(PlatformSemihostParameters* pSemihostParameters);
int __mriSemihost_HandleMbedSemihostRequest(PlatformSemihostParameters* pParameters);

/* Macroes which allow code to drop the __mri namespace prefix. */
#define Semihost_IsDebuggeeMakingSemihostCall   __mriSemihost_IsDebuggeeMakingSemihostCall
#define Semihost_HandleSemihostRequest          __mriSemihost_HandleSemihostRequest
#define Semihost_HandleNewlibSemihostRequest    __mriSemihost_HandleNewlibSemihostRequest
#define Semihost_HandleMbedSemihostRequest      __mriSemihost_HandleMbedSemihostRequest

#endif /* _SEMIHOST_H_ */
