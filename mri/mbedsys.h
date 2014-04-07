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
/* Definition of _sys_*() functions and associated constants implemented in mbed/capi.ar */

#ifndef _MBEDSYS_H_
#define _MBEDSYS_H_

#ifdef __cplusplus
extern "C" {
#endif


/* Types used by functions implemented in mbed.ar */
typedef int FILEHANDLE;

/* File openmode values for mbed _sys_open() */
#define OPENMODE_R      0 
#define OPENMODE_B      1 
#define OPENMODE_PLUS   2 
#define OPENMODE_W      4 
#define OPENMODE_A      8 

/* Functions implemented in mbed.ar */
FILEHANDLE  _sys_open(const char* name, int openmode);
int         _sys_close(FILEHANDLE fh);
int         _sys_write(FILEHANDLE fh, const unsigned char* buf, unsigned len, int mode);
int         _sys_read(FILEHANDLE fh, unsigned char* buf, unsigned len, int mode);
int         _sys_seek(FILEHANDLE fh, long pos); 
long        _sys_flen(FILEHANDLE fh); 
int         _sys_istty(FILEHANDLE fh); 


#ifdef __cplusplus
}
#endif

#endif /* _MBEDSYS_H_ */