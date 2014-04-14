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
/* Constants used by gdb for remote file APIs. */
#ifndef _FILEIO_H_
#define _FILEIO_H_

#define O_RDONLY    0x0
#define O_WRONLY    0x1
#define O_RDWR      0x2
#define O_APPEND    0x8
#define O_CREAT     0x200
#define O_TRUNC     0x400

#define S_IRUSR     0400
#define S_IWUSR     0200
#define S_IRGRP     040
#define S_IWGRP     020
#define S_IROTH     04
#define S_IWOTH     02

#define SEEK_SET    0
#define SEEK_CUR    1
#define SEEK_END    2

#endif /* _FILEIO_H_ */
