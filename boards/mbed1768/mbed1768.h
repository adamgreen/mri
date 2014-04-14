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
/* Routines which expose mbed1768 specific functionality to the mri debugger. */
#ifndef _MBED1768_H_
#define _MBED1768_H_

#define MBED1768_UID_SIZE 36

const uint8_t* __mriMbed1768_GetMbedUid(void);
int            __mriMbed1768_IsMbedDevice(void);

#endif /* _MBED1768_H_ */
