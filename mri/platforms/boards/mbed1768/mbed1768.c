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
#include <string.h>
#include <platforms.h>
#include <try_catch.h>
#include "../../architectures/cortex-m/debug_cm3.h"
#include "../../devices/lpc176x/lpc176x.h"
#include "mbed1768_asm.h"
#include "mbed1768.h"

#define FLAGS_MBED_DETECTED 1

typedef struct
{
    uint32_t flags;
    uint8_t  mbedUid[MBED1768_UID_SIZE];
} Mbed1768State;

static Mbed1768State g_state;


static void initModuleState(void);
static void disableMbedInterface(void);
static void fetchAndSaveMbedUid(void);
static void setMbedDetectedFlag(void);
void Platform_Init(Token* pParameterTokens)
{
    initModuleState();
    
    __try
    {
        __throwing_func( disableMbedInterface() );
        __throwing_func( __mriLpc176x_Init(pParameterTokens) );
    }
    __catch
    {
        __rethrow;
    }
}

static void initModuleState(void)
{
    static const uint8_t  defaultMbedUid[MBED1768_UID_SIZE] = "101000000000000000000002F7F00000\0\0\0";
    
    g_state.flags = 0;
    memcpy(g_state.mbedUid, defaultMbedUid, sizeof(g_state.mbedUid));
}

static void disableMbedInterface(void)
{
    static const uint32_t debugDetachWaitTimeout = 5000;
    
    /* mbed interface exists on JTAG bus so if no debugger, then no potential for mbed interface. */
    if (!isDebuggerAttached())
        return;
    
    fetchAndSaveMbedUid();
    __mriDisableMbed();
    
    __try
        waitForDebuggerToDetach(debugDetachWaitTimeout);
    __catch
        __rethrow;

    setMbedDetectedFlag();
}

static void fetchAndSaveMbedUid(void)
{
    __mriGetMbedUid(g_state.mbedUid);
}

static void setMbedDetectedFlag(void)
{
    g_state.flags |= FLAGS_MBED_DETECTED;
}


const uint8_t* __mriMbed1768_GetMbedUid(void)
{
    return g_state.mbedUid;
}


int __mriMbed1768_IsMbedDevice(void)
{
    return (int)(g_state.flags & FLAGS_MBED_DETECTED);
}
