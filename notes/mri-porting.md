# MRI Porting
MRI already contains most of the code it requires to run on any ARMv7-M based Cortex-M3/M4 processor.  If you are adding support for a new device, you need to provide code which provides the following device specific functionality for MRI:
* code which allows MRI to communicate to GDB via the device's specific UART hardware.
* code which provides the XML that describes the memory layout for this particular device.

This page gives an overview of what is required if a developer wants to add support for another device/board to the MRI debug monitor.

## CMSIS Headers
The MRI project already contains CMSIS headers for the devices that it currently supports in [this folder](../cmsis).  When adding a new device to MRI, you will want to:
* Create a new subfolder under this cmsis/ folder for your new device (i.e. LPC17xx, LPC43xx, etc).
* Copy the vendor provided CMSIS headers for the device into this new directory.  One source for these CMSIS device headers is the [mbed-os project](https://github.com/ARMmbed/mbed-os).
  * Just copy over the **.h** device specific header files.
  * Don't copy over the .c, .s, and toolchain specific folders as MRI doesn't require them.


## New Device Sources
Adding support for a new device typically requires the creation of a new subfolder under the [devices/ folder](../devices).  Using the existing [devices/lpc176x/](../devices/lpc176x) device folder as an example, we see
that it contains the following sources:
```console
/depots/mri/devices/lpc176x$ ls -l
total 80
-rw-r--r--  1 adamgreen  wheel   2069 Jan 28 10:18 lpc176x_asm.S
-rw-r--r--  1 adamgreen  wheel   4647 Feb  1 14:19 lpc176x_init.c
-rw-r--r--  1 adamgreen  wheel   1417 Feb  1 21:54 lpc176x_init.h
-rw-r--r--  1 adamgreen  wheel  18652 Feb  1 14:20 lpc176x_uart.c
-rw-r--r--  1 adamgreen  wheel   1495 Apr 13  2014 lpc176x_uart.h
```
Any new device that is added will require similar files and the following sections provide an overview of what each of
these files contains.

### device_asm.S
This assembly language file provides the default UART interrupt handler implementations.  When initializing the UART to be used for communicating with GDB, it should be configured to issue an interrupt when new bytes are received. This allows a CTRL+C sent from GDB while the firmware is running to generate an interrupt.  This interrupt can then pass control to MRI, halting the firmware and allowing GDB to query the current state of the device.  The priority of this interrupt should also be initialized to 0, giving these GDB interrupts the highest priority possible.  If any interrupt handler runs at a priority equal or higher than the UART, then it can't be broken into if it should hang or otherwise go amiss.

This assembly language file provides a default handler for each UART interrupt supported by the device.  These default handlers will just branch immediately to the ```mriExceptionHandler``` routine.  This routine is implemented by the existing ARMv7-M architecture level code.  The UART handlers are weak implementations so that the developer can provide their own strong implementation for the UARTs they plan to use for their own firmware, leaving the unused ones ready to be used by MRI for debugging.  This is an example implementation from [lpc176x_asm.S](../devices/lpc176x/lpc176x_asm.S):
```assembly
    .global UART0_IRQHandler
    .weak   UART0_IRQHandler
    .type UART0_IRQHandler, function
    .thumb_func
    /* extern "C" void UART0_IRQHandler(void);
       Override UART0 exceptions and send to mriExceptionHandler.
    */
UART0_IRQHandler:
    b       mriExceptionHandler
```

How many of these UART handlers should be provided and what should they be named?  The number depends on the amount of UART ports provided by a particular device.  The names must match those placed in the interrupt vector table by the CMSIS startup code.  The following is an excerpt from the [LPC17xx CMSIS startup code for GCC](https://github.com/ARMmbed/mbed-os/blob/master/targets/TARGET_NXP/TARGET_LPC176X/device/TOOLCHAIN_GCC_ARM/startup_LPC17xx.S):
```assembly
__isr_vector:
    ...
    /* External interrupts */
    .long   WDT_IRQHandler              /* 16: Watchdog Timer               */
    .long   TIMER0_IRQHandler           /* 17: Timer0                       */
    .long   TIMER1_IRQHandler           /* 18: Timer1                       */
    .long   TIMER2_IRQHandler           /* 19: Timer2                       */
    .long   TIMER3_IRQHandler           /* 20: Timer3                       */
    .long   UART0_IRQHandler            /* 21: UART0                        */
    .long   UART1_IRQHandler            /* 22: UART1                        */
    .long   UART2_IRQHandler            /* 23: UART2                        */
    .long   UART3_IRQHandler            /* 24: UART3                        */
    ...
```

This shows us that the handlers should be named **UART0_IRQHandler**, **UART1_IRQHandler**, **UART2_IRQHandler**, and **UART3_IRQHandler** for the LPC176x.

### device_init.h
This header file exposes 3 important pieces of information about this device implementation:
1. Defines **MRI_DEVICE_HAS_FPU** to have a value of 1 if the device contains a FPU and a 0 otherwise.
2. Declares the name of the routine used to initialize this device for MRI.  The board level source code #includes this header file so that it can call this device initialization code from its ```Platform_Init()``` routine.
3. Declares any other device specific state that may need to be shared amongst the device modules and/or with the board module(s) that utilize this device.

An example can be found in [devices/lpc176x/lpc176x_init.h](../devices/lpc176x/lpc176x_init.h).  In this example we see:
* The **MRI_DEVICE_HAS_FPU** macro has been defined to have a value of 0 since the LPC1768 doesn't have a FPU.
* ```mriLpc176x_Init()``` is the name of the routine to be called from the ```Platform_Init()``` to initialize this device.
* ```mriLpc176xState``` is the name of the global state maintained for the LPC176X device code.

**Note:** The externally visible globals have been given names which begin with ```mri```.  All externally visible symbols in the MRI lib should start with this ```mri``` prefix to reduce the chance that they will conflict with the firmware into which it is being linked.  The [globalchk](../tests/globalchk) bash script is used to check that this rule has been honoured.

### device_init.c
This device specific module typically takes care of the following important items:
* Implements ```Platform_GetDeviceMemoryMapXmlSize()``` and ```Platform_GetDeviceMemoryMapXml()``` to return XML which describes the memory layout for this particular device.  More details about what GDB expects in this XML can be found [here](https://sourceware.org/gdb/onlinedocs/gdb/Memory-Map-Format.html).
* Provides the implementation of a device specific init routine.

[devices/lpc176x/lpc176x_init.c](../devices/lpc176x/lpc176x_init.c) provides an example device specific init routine:
```c
/* Reference this handler in the ASM module to make sure that it gets linked in. */
void UART0_IRQHandler(void);


void mriLpc176x_Init(Token* pParameterTokens)
{
    /* Reference handler in ASM module to make sure that is gets linked in. */
    void (* volatile dummyReference)(void) = UART0_IRQHandler;
    (void)dummyReference;

    __try
        mriCortexMInit(pParameterTokens, 0, CANActivity_IRQn);
    __catch
        __rethrow;

    /* mriCortexInit() sets all interrupts to lower priority than debug monitor. Interrupt for UART used by GDB must be
       elevated to the same level as DebugMon_Handler, so initialize it after calling mriCortexInit().
    */
    mriLpc176xUart_Init(pParameterTokens);
}
```

Notes about this example:
* The initial dummy reference in ```mriLpc176x_Init()``` to ```UART0_IRQHandler``` is performed so that code which gets calls from ```mriInit()``` will have a reference to something in device_asm.S.  Without this reference the GNU linker would most likely not bother to link in the UART handlers from this assembly language module and instead use the weak implementations found in the CMSIS startup code. The [nRF52 UART code](../devices/nrf52/nrf52_uart.c) shows another way to do this where the UART handler is included in the same C module as inline assembly.
* Calls ```mriCortexMInit()``` to initialize the ARMv7-M architecture level portion of MRI.  If this should fail then it will return immediately rather than attempting to initialize device specific features. The second parameter of ```0``` is the priority level at which the debugger should run. The third parameter of ```CANActivity_IRQn``` is the highest external device IRQn on the LPC176x. It is used to initialize all of the LPC176x external interrupts to priority 1, reserving priority 0 for debugger related interrupts/exceptions.
* Finally it calls ```mriLpc176xUart_Init()``` to initialize the UART to be used for communicating to GDB.

### device_uart.h / device_uart.c
These files provide device specific code to be used by MRI to communicate with GDB via the UART.  MRI requires that the following communication routines be implemented:
|                               |                                                                                 |
|-------------------------------|---------------------------------------------------------------------------------|
| Platform_CommHasReceiveData() | Returns 0 if no data from GDB has already been received and non-zero otherwise. |
| Platform_CommReceiveChar() | Returns next received character from GDB. This call should block. |
| Platform_CommHasTransmitCompleted() | Returns 0 if the last packet hasn't been completely sent to GDB. |
| Platform_CommSendChar()       | Sends a single byte to GDB. |

If it is more convenient for you to send a whole packet at once rather than a byte at a time (ie. USB or TCP/IP as transport protocol), then you can implement the ```Platform_CommSendBuffer()``` routine as well. ```Platform_CommSendChar()``` will still be called to send single byte ACK/NAK bytes back to GDB:
|                               |                                                                                 |
|-------------------------------|---------------------------------------------------------------------------------|
| Platform_CommSendBuffer()     | Sends a packet to GDB, all at once. |

The existing [devices/lpc176x/lpc176x_uart.h](../devices/lpc176x/lpc176x_uart.h) and [devices/lpc176x/lpc176x_uart.c](../devices/lpc176x/lpc176x_uart.c) provide example implementations.


## New Board Sources
Adding support for a new board typically requires the creation of a new subfolder under the [boards/ folder](../boards).  A board contains a device but it might also provide other debug related features such as additional external memory (in which case it would make sense to have ```Platform_GetDeviceMemoryMapXmlSize()``` and ```Platform_GetDeviceMemoryMapXml()``` implemented at the board level rather than at the device level) or mbed specific features such as the interface chip.

### board.c
The typical board specific module is pretty simple.  The
[Micromint Bambino 210 board module](../boards/bambino210/bambino210.c) is a pretty good example:
```c
void Platform_Init(Token* pParameterTokens)
{
    mriLpc43xx_Init(pParameterTokens);
}


const uint8_t* Platform_GetUid(void)
{
    return NULL;
}


uint32_t Platform_GetUidSize(void)
{
    return 0;
}
```

The main thing it does is implement ```Platform_Init()``` which just calls the device specific initialization routine, ```mriLpc43xx_Init()```.  The mbed interface chip on the LPC1768 board actually contains a UID which is used by the ethernet drivers to obtain the MAC address.  The Bambino doesn't have this so it just returns an empty UID in the above example.


## Debugger Stack
The ```mriCortexMDebuggerStack``` is the stack used while the program is halted in MRI. Its size is set by the **CORTEXM_DEBUGGER_STACK_SIZE** global in [armv7v8-m.h](../architectures/armv7v8-m/armv7v8-m.h) and might need to be increased if some of your **Platform_*** implementations use more stack than the current device implementations. The ```mriCortexMState.maxStackUsed``` global is updated each time execution is resumed. It indicates the maximum amount of debugger stack used so far in bytes.


## Makefile Updates
You will typically need to make two updates to the [MRI makefile](../Makefile) to get it building the new code that you have added.  You need one update to tell make about the new device specific files you have added and then another update to pull everything together and build a new library for your board/device combination.

### Device Updates
You will want to append a section for your new device to this part of the makefile:
```makefile
# ** DEVICES **
# LPC176x device sources.
$(eval $(call armv7m_module,LPC176X,devices/lpc176x))

# LPC43xx device sources.
$(eval $(call armv7m_module,LPC43XX,devices/lpc43xx))
```

When calling the armv7m_module makefile macro, you pass in two parameters:
1. A symbolic name for your device which is unique within this makefile.  It is typically all caps and in the above snippet we see names like **LPC176X** and **LPC43XX** used for this parameter.
2. The directory where your device specific source code is located.

### Board Updates
You will want to append a section for your new board to this part of the makefile:
```makefile
# ** BOARDS **
# mbed 1768 board
$(eval $(call armv7m_module,MBED1768,boards/mbed1768))
$(eval $(call make_board_library,MBED1768_1,libmri_mbed1768.a,\
                                 CORE SEMIHOST ARMV7M NATIVE_MEM LPC176X MBED1768,\
                                 cmsis/LPC17xx))

# Bambino 210 LPC4330 board
$(eval $(call armv7m_module,BAMBINO210,boards/bambino210))
$(eval $(call make_board_library,BAMBINO210_1,libmri_bambino210.a,\
                                 CORE_FPU SEMIHOST_FPU ARMV7M_FPU NATIVE_MEM_FPU LPC43XX_FPU BAMBINO210_FPU,\
                                 cmsis/LPC43xx))
```

When calling the make_board_library makefile macro, you pass in four parameters:
1. A symbolic name for your board which is unique within this makefile.  It is typically all caps and in the above snippet we see names like **MBED1768_1** and **BAMBINO210_1** used for this parameter.
2. The filename to be given to your library (ie. libmri_mbed1768.a).
3. A list of modules to be linked into your board library.  Some notes about this list:
  * You will want to include **CORE SEMIHOST ARMV7M NATIVE_MEM** -or- **CORE_FPU SEMIHOST_FPU ARMV7M_FPU NATIVE_MEM_FPU** depending on whether the device on your board has a FPU or not. The **_FPU_HARD** suffix can also be used if you want to use the floating point registers for passing float parameters into functions instead.
  * You will want to include your device specific module too. The symbolic name to be used will be the one you used as the first parameter to the armv7m_module macro when you added your device to the makefile.  (ie. **LPC176X**, **LPC43XX_FPU**, etc.) You will need to add the _FPU suffix if this device has a FPU.
4. The INCLUDE path to be used when building the modules for this library.  This should include the paths to the CMSIS headers for your device.


## GLOBALCHK Test Script
The [tests/globalchk](../tests/globalchk) bash script is used to check that MRI meets the global symbol restrictions that we want to maintain for the library:
* All of the exported global symbols from the library should begin with the ```mri``` prefix so that they don't collide with the debuggee's globals.
* The library shouldn't import any global symbols. All of its code should be self contained. We don't want the debug monitor and debuggee executing the same code since breakpoints set in the shared code by the debuggee could end up hitting the breakpoint while the debuggee is running which will lead to unexpected behaviour. Standard C Library functions used by MRI have been re-implemented in [libc.c](../core/libc.c) so that the library adheres to this restriction.
