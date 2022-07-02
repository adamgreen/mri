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
/* Routines used to provide NRF52xxx UART functionality to the mri debugger. */
#include <cmsis/NRF52/nrf52.h>
#include "nrf52_init.h"
#include <string.h>
#include <stdlib.h>
#include <core/platforms.h>
#include <architectures/armv7-m/armv7-m.h>


typedef struct
{
    uint32_t    baudRate;
    uint32_t    txPin;
    uint32_t    rxPin;
    uint32_t    priorityLevel;
} UartParameters;


static void     parseUartParameters(Token* pParameterTokens, UartParameters* pParameters);
static void     configureUartForUseOfDebugger(UartParameters* pParameters);
static void     configureNVICForUartInterrupt(uint32_t priorityLevel);
void mriNRF52Uart_Init(Token* pParameterTokens, uint32_t debugMonPriorityLevel)
{
    UartParameters    parameters;

    parseUartParameters(pParameterTokens, &parameters);
    parameters.priorityLevel = debugMonPriorityLevel;
    configureUartForUseOfDebugger(&parameters);
}

static void parseUartParameters(Token* pParameterTokens, UartParameters* pParameters)
{
    static const char baudRatePrefix[] = "MRI_UART_BAUD=";
    static const char txPinPrefix[] = "MRI_UART_TXPIN=";
    static const char rxPinPrefix[] = "MRI_UART_RXPIN=";
    const char*       pMatchingPrefix = NULL;

    /* Default the UART parameters to use 230400 baud on pins used on nRF52-DK. */
    pParameters->baudRate = 230400;
    pParameters->txPin = 6;
    pParameters->rxPin = 8;

    /* Check for user provided overrides for the UART parameters. */
    if ((pMatchingPrefix = Token_MatchingStringPrefix(pParameterTokens, baudRatePrefix)) != NULL)
        pParameters->baudRate = uint32FromString(pMatchingPrefix + sizeof(baudRatePrefix)-1);
    if ((pMatchingPrefix = Token_MatchingStringPrefix(pParameterTokens, txPinPrefix)) != NULL)
        pParameters->txPin = uint32FromString(pMatchingPrefix + sizeof(txPinPrefix)-1);
    if ((pMatchingPrefix = Token_MatchingStringPrefix(pParameterTokens, rxPinPrefix)) != NULL)
        pParameters->rxPin = uint32FromString(pMatchingPrefix + sizeof(rxPinPrefix)-1);
}

static void configureUartForUseOfDebugger(UartParameters* pParameters)
{
    /* Make sure that the UART is disabled before starting to configure it. */
    NRF_UART0->ENABLE = 0;

    /* Make sure that none of the short circuit task/event paths are enabled. */
    NRF_UART0->SHORTS = 0;

    /* Disable hardware flow control and its associated RTS/CTS pins. */
    const uint32_t disconnectedPin = 0xFFFFFFFF;
    NRF_UART0->CONFIG = 0;
    NRF_UART0->PSELRTS = disconnectedPin;
    NRF_UART0->PSELCTS = disconnectedPin;

    /* Use the desired pins for TX and RX. */
    NRF_UART0->PSELTXD = pParameters->txPin;
    NRF_UART0->PSELRXD = pParameters->rxPin;

    /* Use the desired baud rate. Defaults to 230400 value if baud rate not found in table. */
    static struct {
        uint32_t baudRate;
        uint32_t regValue;
    } const baudRateValues[] = {
        { 1200, 0x0004F000 },
        { 2400, 0x0009D000 },
        { 4800, 0x0013B000 },
        { 9600, 0x00275000 },
        { 14400, 0x003B0000 },
        { 19200, 0x004EA000 },
        { 28800, 0x0075F000 },
        { 38400, 0x009D5000 },
        { 57600, 0x00EBF000 },
        { 76800, 0x013A9000 },
        { 115200, 0x01D7E000 },
        { 230400, 0x03AFB000 },
        { 250000, 0x04000000 },
        { 460800, 0x075F7000 },
        { 921600, 0x0EBED000 },
        { 1000000, 0x10000000 } };
    uint32_t baudRegValue = 0x03AFB000;
    size_t i;
    for (i = 0 ; i < sizeof(baudRateValues)/sizeof(baudRateValues[0]) ; i++) {
        if (baudRateValues[i].baudRate == pParameters->baudRate) {
            baudRegValue = baudRateValues[i].regValue;
            break;
        }
    }
    NRF_UART0->BAUDRATE = baudRegValue;

    /* Make sure that the events are cleared. */
    NRF_UART0->EVENTS_RXDRDY = 0;
    NRF_UART0->EVENTS_TXDRDY = 0;

    /* Enable interrupt on received data so that CTRL+C from GDB will break into running process. */
    const uint32_t rxdrdy = 1 << 2;
    NRF_UART0->INTENCLR = 0xFFFFFFFF;
    NRF_UART0->INTENSET = rxdrdy;

    configureNVICForUartInterrupt(pParameters->priorityLevel);

    /* Enable the UART (not UARTE) and start it running. */
    const uint32_t enableUART = 4;
    NRF_UART0->ENABLE = enableUART;
    NRF_UART0->TASKS_STARTRX = 1;
    NRF_UART0->TASKS_STARTTX = 1;
}

static void configureNVICForUartInterrupt(uint32_t priorityLevel)
{
    mriCortexMSetPriority(UARTE0_UART0_IRQn, priorityLevel, 0);
    NVIC_EnableIRQ(UARTE0_UART0_IRQn);
}




uint32_t Platform_CommHasReceiveData(void)
{
    return NRF_UART0->EVENTS_RXDRDY;
}


uint32_t  Platform_CommHasTransmitCompleted(void)
{
    /* Platform_CommSendChar() always waits for byte to be sent before returning so always return true from here. */
    return 1;
}


static void waitForUartToReceiveData(void);
int Platform_CommReceiveChar(void)
{
    waitForUartToReceiveData();

    /* Clear event first and then return received byte. */
    NRF_UART0->EVENTS_RXDRDY = 0;
    return (int)NRF_UART0->RXD;
}

static void waitForUartToReceiveData(void)
{
    while (!Platform_CommHasReceiveData())
    {
    }
}

static void waitForTransmitToComplete(void);
static uint32_t hasTransmitCompleted(void);
void Platform_CommSendChar(int Character)
{
    NRF_UART0->TXD = (uint8_t)Character;
    waitForTransmitToComplete();
}

static void waitForTransmitToComplete(void)
{
    while (!hasTransmitCompleted())
    {
    }
    NRF_UART0->EVENTS_TXDRDY = 0;
}

static uint32_t hasTransmitCompleted(void)
{
    return NRF_UART0->EVENTS_TXDRDY;
}


/* Implementation of nRF52xxx UART0 ISR to be intercepted and sent to mri instead. */
void __attribute__((naked)) UARTE0_UART0_IRQHandler(void)
{
    __asm volatile (
    "   .syntax unified                 \n"
    "   b.w       mriExceptionHandler   \n"
    );
}