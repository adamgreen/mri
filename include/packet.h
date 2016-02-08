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
/* 'Class' to manage the sending and receiving of packets to/from gdb.  Takes care of crc and ack/nak handling too. */
#ifndef _PACKET_H_
#define _PACKET_H_

#include <stdio.h>
#include "buffer.h"

typedef struct
{
    Buffer*        pBuffer;
    char           lastChar;
    unsigned char  calculatedChecksum;
    unsigned char  expectedChecksum;
} Packet;

/* Real name of functions are in __mri namespace. */
void    __mriPacket_GetFromGDB(Packet* pPacket, Buffer* pBuffer);
void    __mriPacket_SendToGDB(Packet* pPacket, Buffer* pBuffer);

/* Macroes which allow code to drop the __mri namespace prefix. */
#define Packet_Init         __mriPacket_Init
#define Packet_GetFromGDB   __mriPacket_GetFromGDB
#define Packet_SendToGDB    __mriPacket_SendToGDB


#endif /* _PACKET_H_ */
