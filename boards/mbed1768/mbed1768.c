/* Copyright 2022 Adam Green (https://github.com/adamgreen/)

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/
/* Routines which expose mbed1768 specific functionality to the mri debugger. */
#include <core/libc.h>
#include <core/platforms.h>
#include <core/try_catch.h>
#include <architectures/armv7-m/debug_cm3.h>
#include <devices/lpc176x/lpc176x_init.h>
#include "mbed1768_asm.h"

#define FLAGS_MBED_DETECTED 1

#define MBED1768_UID_SIZE   36

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
        __throwing_func( mriLpc176x_Init(pParameterTokens) );
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
    mri_memcpy(g_state.mbedUid, defaultMbedUid, sizeof(g_state.mbedUid));
}

static void disableMbedInterface(void)
{
    static const uint32_t debugDetachWaitTimeout = 5000;

    /* mbed interface exists on JTAG bus so if no debugger, then no potential for mbed interface. */
    if (!isDebuggerAttached())
        return;

    fetchAndSaveMbedUid();
    mriDisableMbed();

    __try
        waitForDebuggerToDetach(debugDetachWaitTimeout);
    __catch
        __rethrow;

    setMbedDetectedFlag();
}

static void fetchAndSaveMbedUid(void)
{
    mriGetMbedUid(g_state.mbedUid);
}

static void setMbedDetectedFlag(void)
{
    g_state.flags |= FLAGS_MBED_DETECTED;
}


const uint8_t* Platform_GetUid(void)
{
    return g_state.mbedUid;
}


uint32_t Platform_GetUidSize(void)
{
    return sizeof(g_state.mbedUid);
}


int mriMbed1768_IsMbedDevice(void)
{
    return (int)(g_state.flags & FLAGS_MBED_DETECTED);
}
