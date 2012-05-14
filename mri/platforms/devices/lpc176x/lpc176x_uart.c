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
/* Routines used to provide LPC176x UART functionality to the mri debugger. */
#include "platforms.h"
#include "../../architectures/cortex-m/debug_cm3.h"
#include "lpc176x.h"


static UartConfiguration g_uartConfigurations[] =
{
    {
        &(LPC_SC->PCLKSEL0),
        &(LPC_PINCON->PINSEL0),
        &(LPC_PINCON->PINSEL0),
        (LPC_UART_TypeDef*)LPC_UART0,
        1 << 3,
        3 << 6,
        3 << 4,
        3 << 6,
        0x55555555
    },
    {
        &(LPC_SC->PCLKSEL0),
        &(LPC_PINCON->PINSEL0),
        &(LPC_PINCON->PINSEL1),
        (LPC_UART_TypeDef*)LPC_UART1,
        1 << 4,
        3 << 8,
        3 << 30,
        3 << 0,
        0x55555555
    },
    {
        &(LPC_SC->PCLKSEL1),
        &(LPC_PINCON->PINSEL0),
        &(LPC_PINCON->PINSEL0),
        (LPC_UART_TypeDef*)LPC_UART2,
        1 << 24,
        3 << 16,
        3 << 20,
        3 << 22,
        0x55555555
    },
    {
        &(LPC_SC->PCLKSEL1),
        &(LPC_PINCON->PINSEL0),
        &(LPC_PINCON->PINSEL0),
        (LPC_UART_TypeDef*)LPC_UART3,
        1 << 25,
        3 << 18,
        3 << 0,
        3 << 2,
        0xAAAAAAAA
    }
};


static void     parseUartParameters(Token* pParameterTokens);
static void     saveUartToBeUsedByDebugger(uint32_t mriCommSetting);
static int      isSharingUartWithApplication(void);
static void     configureUartForExclusiveUseOfDebugger(void);
static void     enablePowerToUart(void);
static void     setUartPeripheralClockTo1xCCLK(void);
static uint32_t calculate1xPeripheralClockBits(uint32_t peripheralClockSelectionBitmask);
static void     clearUartFractionalBaudDivisor(void);
static void     enableUartFifoAndDisableDma(void);
static void     setUartTo8N1(void);
static void     selectUartPins(void);
static void     selectPinForTx(void);
static void     selectPinForRx(void);
static uint32_t calculatePinSelectionValue(uint32_t originalPinSelectionValue, uint32_t mask);
static void     enableUartToInterruptOnReceivedChar(void);
static void     initUartAutoBaudDetection(void);
static void     configureNVICForUartInterrupt(void);
static int      calculateUartIndex(void);
void __mriLpc176xUart_Init(Token* pParameterTokens)
{
    parseUartParameters(pParameterTokens);

    if (!isSharingUartWithApplication())
        configureUartForExclusiveUseOfDebugger();
}

static void parseUartParameters(Token* pParameterTokens)
{
    uint32_t uartIndex = 0;
    
    if (Token_MatchingString(pParameterTokens, "MRI_UART_MBED_USB"))
        uartIndex = 0;
    if (Token_MatchingString(pParameterTokens, "MRI_UART_MBED_P9_P10"))
        uartIndex = 3;
    if (Token_MatchingString(pParameterTokens, "MRI_UART_MBED_P13_P14"))
        uartIndex = 1;
    if (Token_MatchingString(pParameterTokens, "MRI_UART_MBED_P28_P27"))
        uartIndex = 2;
    saveUartToBeUsedByDebugger(uartIndex);
    
    __mriLpc176xState.flags = 0;
    if (Token_MatchingString(pParameterTokens, "MRI_UART_SHARE"))
        __mriLpc176xState.flags |= LPC176X_UART_FLAGS_SHARE;
}

static void saveUartToBeUsedByDebugger(uint32_t mriUart)
{
    __mriLpc176xState.pCurrentUart = &g_uartConfigurations[mriUart];
}

static int isSharingUartWithApplication(void)
{
    return __mriLpc176xState.flags & LPC176X_UART_FLAGS_SHARE;
}

static void configureUartForExclusiveUseOfDebugger(void)
{
    enablePowerToUart();
    setUartPeripheralClockTo1xCCLK();
    clearUartFractionalBaudDivisor();
    enableUartFifoAndDisableDma();
    setUartTo8N1();
    selectUartPins();
    enableUartToInterruptOnReceivedChar();
    initUartAutoBaudDetection();
    configureNVICForUartInterrupt();
}

static void enablePowerToUart(void)
{
    LPC_SC->PCONP |= __mriLpc176xState.pCurrentUart->powerConfigurationBit;
}

static void setUartPeripheralClockTo1xCCLK(void)
{
    *__mriLpc176xState.pCurrentUart->pPeripheralClockSelection &= 
                        ~__mriLpc176xState.pCurrentUart->peripheralClockSelectionBitmask;
    *__mriLpc176xState.pCurrentUart->pPeripheralClockSelection |= 
                        calculate1xPeripheralClockBits(__mriLpc176xState.pCurrentUart->peripheralClockSelectionBitmask);
}

static uint32_t calculate1xPeripheralClockBits(uint32_t peripheralClockSelectionBitmask)
{
    static const uint32_t CCLK1xForAllPeripherals = 0x55555555;
    
    return (CCLK1xForAllPeripherals & peripheralClockSelectionBitmask);
}

static void clearUartFractionalBaudDivisor(void)
{
    __mriLpc176xState.pCurrentUart->pUartRegisters->FDR = 0x10;
}

static void enableUartFifoAndDisableDma(void)
{
    static const uint32_t enableFifoDisableDmaSetReceiveInterruptThresholdTo0 = 0x01;
    
    __mriLpc176xState.pCurrentUart->pUartRegisters->FCR = enableFifoDisableDmaSetReceiveInterruptThresholdTo0;
}

static void setUartTo8N1(void)
{
    static const uint8_t wordLength8Bit = 0x3;
    static const uint8_t stopBit1 = 0 << 2;
    static const uint8_t disableParity = 0 << 3;
    static const uint8_t lineControlValueFor8N1 = wordLength8Bit | disableParity | stopBit1;
    
    __mriLpc176xState.pCurrentUart->pUartRegisters->LCR = lineControlValueFor8N1;
}

static void selectUartPins(void)
{
    selectPinForTx();
    selectPinForRx();
}

static void selectPinForTx(void)
{
    *__mriLpc176xState.pCurrentUart->pTxPinSelection = 
                                    calculatePinSelectionValue(*__mriLpc176xState.pCurrentUart->pTxPinSelection, 
                                                               __mriLpc176xState.pCurrentUart->txPinSelectionMask);
}

static void selectPinForRx(void)
{
    *__mriLpc176xState.pCurrentUart->pRxPinSelection = 
                                    calculatePinSelectionValue(*__mriLpc176xState.pCurrentUart->pRxPinSelection, 
                                                               __mriLpc176xState.pCurrentUart->rxPinSelectionMask);
}

static uint32_t calculatePinSelectionValue(uint32_t originalPinSelectionValue, uint32_t mask)
{
    uint32_t pinSelectionValue = originalPinSelectionValue;
    
    pinSelectionValue &= ~mask;
    pinSelectionValue |= __mriLpc176xState.pCurrentUart->pinSelectionValue & mask;
    
    return pinSelectionValue;
}

static void enableUartToInterruptOnReceivedChar(void)
{
    static const uint32_t baudDivisorLatchBit = (1 << 7);
    static const uint32_t enableReceiveDataInterrupt = (1 << 0);
    uint32_t              originalLCR;
    
    originalLCR = __mriLpc176xState.pCurrentUart->pUartRegisters->LCR;
    __mriLpc176xState.pCurrentUart->pUartRegisters->LCR &= ~baudDivisorLatchBit;
    __mriLpc176xState.pCurrentUart->pUartRegisters->IER = enableReceiveDataInterrupt;
    __mriLpc176xState.pCurrentUart->pUartRegisters->LCR = originalLCR;
}

static void initUartAutoBaudDetection(void)
{
    static const uint32_t   autoBaudStart = 1;
    static const uint32_t   autoBaudModeForStartBitOnly = 1 << 1;
    static const uint32_t   autoBaudAutoRestart = 1 << 2;
    static const uint32_t   autoBaudValue = autoBaudStart | autoBaudModeForStartBitOnly | autoBaudAutoRestart;
    
    __mriLpc176xState.pCurrentUart->pUartRegisters->ACR = autoBaudValue;
}

static void configureNVICForUartInterrupt(void)
{
    IRQn_Type uart0BaseIRQ = UART0_IRQn;
    IRQn_Type currentUartIRQ;
    
    currentUartIRQ = (IRQn_Type)((int)uart0BaseIRQ + calculateUartIndex());
    NVIC_SetPriority(currentUartIRQ, 0);
    NVIC_EnableIRQ(currentUartIRQ);
}

static int calculateUartIndex(void)
{
    return __mriLpc176xState.pCurrentUart - g_uartConfigurations;
}


uint32_t Platform_CommHasReceiveData(void)
{
    static const uint8_t receiverDataReadyBit = 1 << 0;
    
    return __mriLpc176xState.pCurrentUart->pUartRegisters->LSR & receiverDataReadyBit;
}


static void     waitForUartToReceiveData(void);
int Platform_CommReceiveChar(void)
{
    waitForUartToReceiveData();

    return (int)__mriLpc176xState.pCurrentUart->pUartRegisters->RBR;
}

static void waitForUartToReceiveData(void)
{
    while (!Platform_CommHasReceiveData())
    {
    }
}

static void     waitForUartToAllowTransmit(void);
static uint32_t targetUartCanTransmit(void);
void Platform_CommSendChar(int Character)
{
    waitForUartToAllowTransmit();
    
    __mriLpc176xState.pCurrentUart->pUartRegisters->THR = (uint8_t)Character;
}

static void waitForUartToAllowTransmit(void)
{
    while (!targetUartCanTransmit())
    {
    }
}

static uint32_t targetUartCanTransmit(void)
{
    static const uint8_t transmitterHoldRegisterEmptyBit = 1 << 5;
    
    return __mriLpc176xState.pCurrentUart->pUartRegisters->LSR & transmitterHoldRegisterEmptyBit;
}


int Platform_CommCausedInterrupt(void)
{
    const uint32_t uart0BaseExceptionId = 21;
    uint32_t       interruptSource = getCurrentlyExecutingExceptionNumber();
    uint32_t       currentUartExceptionId;
    
    currentUartExceptionId = uart0BaseExceptionId + calculateUartIndex();
    return interruptSource == currentUartExceptionId;
}


void Platform_CommClearInterrupt(void)
{
    uint32_t interruptId;
    
    interruptId = __mriLpc176xState.pCurrentUart->pUartRegisters->IIR;
    (void)interruptId;
}


int Platform_CommIsSharedWithApplication(void)
{
    return (int)(__mriLpc176xState.flags & LPC176X_UART_FLAGS_SHARE);
}


int Platform_CommIsWaitingForGdbToConnect(void)
{
    static const uint32_t autoBaudStarting = 1;
    
    if (isSharingUartWithApplication())
        return 0;

    return (int)(__mriLpc176xState.pCurrentUart->pUartRegisters->ACR & autoBaudStarting);
}
