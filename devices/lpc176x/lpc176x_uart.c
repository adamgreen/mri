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
/* Routines used to provide LPC176x UART functionality to the mri debugger. */
#include <stdlib.h>
#include <core/libc.h>
#include <core/platforms.h>
#include "lpc176x_init.h"
#include <architectures/armv7v8-m/armv7v8-m.h>
#include <architectures/armv7v8-m/debug_cm3.h>


static const UartConfiguration g_uartConfigurations[] =
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

typedef struct
{
    uint32_t uartIndex;
    uint32_t baudRate;
} UartParameters;

typedef struct
{
    uint32_t    integerBaudRateDivisor;
    uint32_t    fractionalBaudRateDivisor;
} BaudRateDivisors;

typedef struct
{
    uint32_t    desiredRatio;
    uint32_t    mul;
    uint32_t    divAdd;
    uint32_t    closestDivisor;
    uint32_t    closestMul;
    uint32_t    closestDivAdd;
    uint32_t    closestDelta;
} CalculateDivisors;


static void     parseUartParameters(Token* pParameterTokens, UartParameters* pParameters);
static void     saveUartToBeUsedByDebugger(uint32_t mriCommSetting);
static uint32_t uint32FromString(const char* pString);
static uint32_t getDecimalDigit(char currChar);
static void     configureUartForExclusiveUseOfDebugger(UartParameters* pParameters);
static void     enablePowerToUart(void);
static void     setUartPeripheralClockTo1xCCLK(void);
static uint32_t calculate1xPeripheralClockBits(uint32_t peripheralClockSelectionBitmask);
static void     clearUartFractionalBaudDivisor(void);
static void     enableUartFifoAndDisableDma(void);
static void     setUartTo8N1(void);
static void     setUartBaudRate(UartParameters* pParameters);
static void     setDivisors(BaudRateDivisors* pDivisors);
static void     setDivisorLatchBit(void);
static void     clearDivisorLatchBit(void);
static BaudRateDivisors calculateBaudRateDivisors(uint32_t baudRate, uint32_t peripheralRate);
static void     initCalculateDivisorsStruct(CalculateDivisors* pThis, uint32_t baudRate, uint32_t peripheralRate);
static uint32_t fixupPeripheralRateFor16XOversampling(uint32_t actualPeripheralRate);
static int      isNoFractionalDivisorRequired(CalculateDivisors* pThis);
static BaudRateDivisors closestDivisors(CalculateDivisors* pThis);
static BaudRateDivisors calculateFractionalBaudRateDivisors(CalculateDivisors* pThis);
static void     checkTheseFractionalDivisors(CalculateDivisors* pThis);
static void     selectUartPins(void);
static void     selectPinForTx(void);
static void     selectPinForRx(void);
static uint32_t calculatePinSelectionValue(uint32_t originalPinSelectionValue, uint32_t mask);
static void     enableUartToInterruptOnReceivedChar(void);
static void     configureNVICForUartInterrupt(void);
static int      commUartIndex(void);
void mriLpc176xUart_Init(Token* pParameterTokens)
{
    UartParameters parameters;

    parseUartParameters(pParameterTokens, &parameters);
    saveUartToBeUsedByDebugger(parameters.uartIndex);
    configureUartForExclusiveUseOfDebugger(&parameters);
}

static void parseUartParameters(Token* pParameterTokens, UartParameters* pParameters)
{
    static const char baudRatePrefix[] = "MRI_UART_BAUD=";
    const char*       pMatchingPrefix = NULL;

    mri_memset(pParameters, 0, sizeof(*pParameters));

    if (Token_MatchingString(pParameterTokens, "MRI_UART_MBED_USB"))
        pParameters->uartIndex = 0;
    if (Token_MatchingString(pParameterTokens, "MRI_UART_MBED_P9_P10"))
        pParameters->uartIndex = 3;
    if (Token_MatchingString(pParameterTokens, "MRI_UART_MBED_P13_P14"))
        pParameters->uartIndex = 1;
    if (Token_MatchingString(pParameterTokens, "MRI_UART_MBED_P28_P27"))
        pParameters->uartIndex = 2;
    if (Token_MatchingString(pParameterTokens, "MRI_UART_0"))
        pParameters->uartIndex = 0;
    if (Token_MatchingString(pParameterTokens, "MRI_UART_1"))
        pParameters->uartIndex = 1;
    if (Token_MatchingString(pParameterTokens, "MRI_UART_2"))
        pParameters->uartIndex = 2;
    if (Token_MatchingString(pParameterTokens, "MRI_UART_3"))
        pParameters->uartIndex = 3;

    if ((pMatchingPrefix = Token_MatchingStringPrefix(pParameterTokens, baudRatePrefix)) != NULL)
        pParameters->baudRate = uint32FromString(pMatchingPrefix + sizeof(baudRatePrefix)-1);
    else
        pParameters->baudRate = 230400;

}

static void saveUartToBeUsedByDebugger(uint32_t mriUart)
{
    mriLpc176xState.pCurrentUart = &g_uartConfigurations[mriUart];
}

static uint32_t uint32FromString(const char* pString)
{
    uint32_t value = 0;

    while (*pString)
    {
        uint32_t digit;

        __try
        {
            digit = getDecimalDigit(*pString++);
        }
        __catch
        {
            clearExceptionCode();
            break;
        }

        value = value * 10 + digit;
    }

    return value;
}

static uint32_t getDecimalDigit(char currChar)
{
    if (currChar >= '0' && currChar <= '9')
        return currChar - '0';
    else
        __throw_and_return(invalidDecDigitException, 0);
}

static void configureUartForExclusiveUseOfDebugger(UartParameters* pParameters)
{
    enablePowerToUart();
    setUartPeripheralClockTo1xCCLK();
    clearUartFractionalBaudDivisor();
    enableUartFifoAndDisableDma();
    setUartTo8N1();
    setUartBaudRate(pParameters);
    selectUartPins();
    enableUartToInterruptOnReceivedChar();
    configureNVICForUartInterrupt();
}

static void enablePowerToUart(void)
{
    LPC_SC->PCONP |= mriLpc176xState.pCurrentUart->powerConfigurationBit;
}

static void setUartPeripheralClockTo1xCCLK(void)
{
    *mriLpc176xState.pCurrentUart->pPeripheralClockSelection &=
                        ~mriLpc176xState.pCurrentUart->peripheralClockSelectionBitmask;
    *mriLpc176xState.pCurrentUart->pPeripheralClockSelection |=
                        calculate1xPeripheralClockBits(mriLpc176xState.pCurrentUart->peripheralClockSelectionBitmask);
}

static uint32_t calculate1xPeripheralClockBits(uint32_t peripheralClockSelectionBitmask)
{
    static const uint32_t CCLK1xForAllPeripherals = 0x55555555;

    return (CCLK1xForAllPeripherals & peripheralClockSelectionBitmask);
}

static void clearUartFractionalBaudDivisor(void)
{
    mriLpc176xState.pCurrentUart->pUartRegisters->FDR = 0x10;
}

static void enableUartFifoAndDisableDma(void)
{
    static const uint32_t enableFifoDisableDmaSetReceiveInterruptThresholdTo0 = 0x01;

    mriLpc176xState.pCurrentUart->pUartRegisters->FCR = enableFifoDisableDmaSetReceiveInterruptThresholdTo0;
}

static void setUartTo8N1(void)
{
    static const uint8_t wordLength8Bit = 0x3;
    static const uint8_t stopBit1 = 0 << 2;
    static const uint8_t disableParity = 0 << 3;
    static const uint8_t lineControlValueFor8N1 = wordLength8Bit | disableParity | stopBit1;

    mriLpc176xState.pCurrentUart->pUartRegisters->LCR = lineControlValueFor8N1;
}

static void setUartBaudRate(UartParameters* pParameters)
{
    BaudRateDivisors  divisors;

    divisors = calculateBaudRateDivisors(pParameters->baudRate, SystemCoreClock);
    setDivisors(&divisors);
}

static BaudRateDivisors calculateBaudRateDivisors(uint32_t baudRate, uint32_t peripheralRate)
{
    CalculateDivisors   calcDivisors;

    initCalculateDivisorsStruct(&calcDivisors, baudRate, peripheralRate);
    if (isNoFractionalDivisorRequired(&calcDivisors))
        return closestDivisors(&calcDivisors);
    return calculateFractionalBaudRateDivisors(&calcDivisors);
}

static void initCalculateDivisorsStruct(CalculateDivisors* pThis, uint32_t baudRate, uint32_t peripheralRate)
{
    /* Calculate desired clock to baud ratio in 17.15 fixed format.
       This code can handle peripheralRates which are less <512MHz since
         2^29 / 16 = 2^29 / 2^4 = 2^25 and (2^25 << 7) fits in 32-bit value.
       If you divide by a low baud rate like 300 (>2^8) then mantissa only needs 25 - 8 = 17 bits. */
    pThis->desiredRatio = ((fixupPeripheralRateFor16XOversampling(peripheralRate) << 7) / baudRate) << 8;
    pThis->mul = 1;
    pThis->divAdd = 0;
    pThis->closestDelta = ~0U;
    pThis->closestDivisor = pThis->desiredRatio >> 15;
    pThis->closestMul = 1;
    pThis->closestDivAdd = 0;
}

static uint32_t fixupPeripheralRateFor16XOversampling(uint32_t actualPeripheralRate)
{
    return actualPeripheralRate / 16;
}

static int isNoFractionalDivisorRequired(CalculateDivisors* pThis)
{
    /* Check for no fractional bits in the 17.15 fixed format. */
    return (pThis->desiredRatio & 0x7FFF) == 0;
}

static BaudRateDivisors closestDivisors(CalculateDivisors* pThis)
{
    BaudRateDivisors divisors;

    divisors.integerBaudRateDivisor = pThis->closestDivisor;
    divisors.fractionalBaudRateDivisor = (pThis->closestMul << 4) | pThis->closestDivAdd;

    return divisors;
}

static BaudRateDivisors calculateFractionalBaudRateDivisors(CalculateDivisors* pThis)
{
    for (pThis->mul = 1 ; pThis->mul <= 15 ; pThis->mul++)
    {
        for (pThis->divAdd = 1 ; pThis->divAdd < pThis->mul ; pThis->divAdd++)
            checkTheseFractionalDivisors(pThis);
    }
    return closestDivisors(pThis);
}

static void checkTheseFractionalDivisors(CalculateDivisors* pThis)
{
    static const uint32_t fixedOne = (1 << 15);
    uint32_t              fixedScale;
    uint32_t              testDivisor;
    uint32_t              fixedRatio;
    uint32_t              fixedDelta;

    fixedScale = fixedOne + ((pThis->divAdd << 15) / pThis->mul);
    testDivisor = pThis->desiredRatio / fixedScale;
    fixedRatio = testDivisor * fixedScale;
    fixedDelta = abs((int32_t)fixedRatio - (int32_t)pThis->desiredRatio);

    if (fixedDelta < pThis->closestDelta)
    {
        pThis->closestDelta = fixedDelta;
        pThis->closestDivisor = testDivisor;
        pThis->closestMul = pThis->mul;
        pThis->closestDivAdd = pThis->divAdd;
    }
}

static void setDivisors(BaudRateDivisors* pDivisors)
{
    LPC_UART_TypeDef* pUartRegisters = mriLpc176xState.pCurrentUart->pUartRegisters;

    setDivisorLatchBit();

    pUartRegisters->DLL = pDivisors->integerBaudRateDivisor & 0xFF;
    pUartRegisters->DLM = pDivisors->integerBaudRateDivisor >> 8;
    pUartRegisters->FDR = pDivisors->fractionalBaudRateDivisor;

    clearDivisorLatchBit();
}

#define LPC176x_UART_LCR_DLAB (1 << 7)

static void setDivisorLatchBit(void)
{
    mriLpc176xState.pCurrentUart->pUartRegisters->LCR |= LPC176x_UART_LCR_DLAB;
}

static void clearDivisorLatchBit(void)
{
    mriLpc176xState.pCurrentUart->pUartRegisters->LCR &= ~LPC176x_UART_LCR_DLAB;
}

static void selectUartPins(void)
{
    selectPinForTx();
    selectPinForRx();
}

static void selectPinForTx(void)
{
    *mriLpc176xState.pCurrentUart->pTxPinSelection =
                                    calculatePinSelectionValue(*mriLpc176xState.pCurrentUart->pTxPinSelection,
                                                               mriLpc176xState.pCurrentUart->txPinSelectionMask);
}

static void selectPinForRx(void)
{
    *mriLpc176xState.pCurrentUart->pRxPinSelection =
                                    calculatePinSelectionValue(*mriLpc176xState.pCurrentUart->pRxPinSelection,
                                                               mriLpc176xState.pCurrentUart->rxPinSelectionMask);
}

static uint32_t calculatePinSelectionValue(uint32_t originalPinSelectionValue, uint32_t mask)
{
    uint32_t pinSelectionValue = originalPinSelectionValue;

    pinSelectionValue &= ~mask;
    pinSelectionValue |= mriLpc176xState.pCurrentUart->pinSelectionValue & mask;

    return pinSelectionValue;
}

static void enableUartToInterruptOnReceivedChar(void)
{
    static const uint32_t baudDivisorLatchBit = (1 << 7);
    static const uint32_t enableReceiveDataInterrupt = (1 << 0);
    uint32_t              originalLCR;

    originalLCR = mriLpc176xState.pCurrentUart->pUartRegisters->LCR;
    mriLpc176xState.pCurrentUart->pUartRegisters->LCR &= ~baudDivisorLatchBit;
    mriLpc176xState.pCurrentUart->pUartRegisters->IER = enableReceiveDataInterrupt;
    mriLpc176xState.pCurrentUart->pUartRegisters->LCR = originalLCR;
}

static void configureNVICForUartInterrupt(void)
{
    IRQn_Type uart0BaseIRQ = UART0_IRQn;
    IRQn_Type currentUartIRQ;

    currentUartIRQ = (IRQn_Type)((int)uart0BaseIRQ + commUartIndex());
    mriCortexMSetPriority(currentUartIRQ, 0, 0);
    NVIC_EnableIRQ(currentUartIRQ);
}


static int commUartIndex(void)
{
    return mriLpc176xState.pCurrentUart - g_uartConfigurations;
}


uint32_t Platform_CommHasReceiveData(void)
{
    static const uint8_t receiverDataReadyBit = 1 << 0;

    return mriLpc176xState.pCurrentUart->pUartRegisters->LSR & receiverDataReadyBit;
}


uint32_t  Platform_CommHasTransmitCompleted(void)
{
    static const uint8_t transmitterEmpty = 1 << 6;

    return mriLpc176xState.pCurrentUart->pUartRegisters->LSR & transmitterEmpty;
}


static void yieldUartBusToDma(void);
static void waitForUartToReceiveData(void);
int Platform_CommReceiveChar(void)
{
    waitForUartToReceiveData();
    yieldUartBusToDma();

    return (int)mriLpc176xState.pCurrentUart->pUartRegisters->RBR;
}

static void waitForUartToReceiveData(void)
{
    while (!Platform_CommHasReceiveData())
    {
        yieldUartBusToDma();
    }
}

static void yieldUartBusToDma(void)
{
    __NOP();
    __NOP();
    __NOP();
}


static void     waitForUartToAllowTransmit(void);
static uint32_t targetUartCanTransmit(void);
void Platform_CommSendChar(int Character)
{
    waitForUartToAllowTransmit();
    yieldUartBusToDma();

    mriLpc176xState.pCurrentUart->pUartRegisters->THR = (uint8_t)Character;
}

static void waitForUartToAllowTransmit(void)
{
    while (!targetUartCanTransmit())
    {
        yieldUartBusToDma();
    }
}

static uint32_t targetUartCanTransmit(void)
{
    static const uint8_t transmitterHoldRegisterEmptyBit = 1 << 5;

    return mriLpc176xState.pCurrentUart->pUartRegisters->LSR & transmitterHoldRegisterEmptyBit;
}
