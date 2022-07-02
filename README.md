## MRI - Monitor for Remote Inspection
MRI is a debug monitor which allows the GNU debugger, GDB, to debug Cortex-M3/M4 processors. This makes it possible to debug applications running on Cortex-M devices using a full featured source level debugger with no extra hardware other than a serial connection.

**Please Note:**  This project just contains the sources to build the MRI debug monitor library but doesn't show how to link it into your program and make use of it.  Such an example is provided by the [GCC4MBED project.](https://github.com/adamgreen/gcc4mbed)


## MRI Features
* 6+ hardware breakpoints (actual number depends on device)
* 4+ data watchpoints (actual number depends on device)
* single stepping
* runs over any of the UART ports on the device (selected when user compiles their code)
* baud rate is determined at runtime (through GDB command line) on devices that support auto-baud detection
* semi-host functionality:
  * stdout/stderr/stdin are redirected to/from the GDB console
  * mbed LocalFileSystem semi-host support (fopen, fwrite, fread, fseek, and fclose) - **mbed-LPC1768 only**
  * maintains access to mbed device's unique ethernet address - **mbed-LPC1768 only**
* works with free [GNU Tools for ARM Embedded Processors](https://launchpad.net/gcc-arm-embedded)
* no program binary size limitations
* open source (Apache Licensed)


## Devices Supported
| Device      | Sample Boards |
|-------------|---------------|
| NXP LPC17xx  | [mbed-1768](https://developer.mbed.org/platforms/mbed-LPC1768/) |
|              | [LPC1769 LPCXpresso Board](https://www.embeddedartists.com/products/lpcxpresso/lpc1769_xpr.php) |
| NXP LPC43xx  | [Micromint Bambino 210](http://www.micromint.com/index.php?option=com_content&view=article&id=199:bambino210&catid=53:products) |
| STM32F429XX  | [STM32F429 Discovery kit](http://www.st.com/web/catalog/tools/FM116/SC959/SS1532/PF259090) |


## How to Clone
This project uses submodules (CppUTest for unit testing).  Cloning requires a few more steps to get all of the necessary
code.

```
git clone git@github.com:adamgreen/mri.git
cd mri
git submodule init
git submodule update
```

**- or -**

```
git clone --recursive git@github.com:adamgreen/mri.git
```


## More MRI Information
**[Getting started with MRI in GCC4MBED](notes/mri-getting-started.creole#installation):**  Documents how to setup the mbed device and GCC4MBED project to use MRI for debugging binaries.  Also provides a walk through of a debugging session with one of the GCC4MBED samples.

**[Supported Host Platforms](notes/mri-platforms.creole#supported-host-platforms):**  Which platforms has GDB been run on while connected to MRI.

**[Supported Devices](notes/mri-devices.creole#supported-devices):**  Which devices can currently be debugged with MRI.

**[Why use MRI](notes/mri-pros-cons.creole):**  Documents the advantages of using a debug monitor like MRI and its known limitations.

**[Porting MRI](notes/mri-porting.md):**  Notes on how to port MRI to new devices.

**[Reporting a Problem](notes/mri-report-problem.creole#hit-a-problem):**  How to extract information from your PC to show what was happening at the time a problem occurs in a MRI debugging session.

**[FAQ](notes/mri-faq.creole#frequently-asked-questions)**
