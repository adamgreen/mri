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
#include <string.h>
#include <stdlib.h>
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

typedef struct
{
    int      share;
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
    uint32_t    targetBaudRate;
    uint32_t    adjustedPeripheralRate;
    uint32_t    mul;
    uint32_t    divAdd;
    uint32_t    closestDivisor;
    uint32_t    closestMul;
    uint32_t    closestDivAdd;
    uint32_t    closestDelta;
} CalculateDivisors;


static void     parseUartParameters(Token* pParameterTokens, UartParameters* pParameters);
static void     saveUartToBeUsedByDebugger(uint32_t mriCommSetting);
static void     setUartSharedFlag(void);
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
static void     setManualBaudFlag(void);
static BaudRateDivisors calculateBaudRateDivisors(uint32_t baudRate, uint32_t peripheralRate);
static void     initCalculateDivisorsStruct(CalculateDivisors* pThis, uint32_t baudRate, uint32_t peripheralRate);
static uint32_t fixupPeripheralRateFor16XOversampling(uint32_t actualPeripheralRate);
static int      isNoFractionalDivisorRequired(CalculateDivisors* pThis);
static BaudRateDivisors closestDivisors(CalculateDivisors* pThis);
static BaudRateDivisors calculateFractionalBaudRateDivisors(CalculateDivisors* pThis);
static void     checkTheseFractionalDivisors(CalculateDivisors* pThis);
static uint32_t calculateIntegerDivisorForTheseFractionalDivisors(CalculateDivisors* pThis);
static uint32_t calculateBaudRate(CalculateDivisors* pThis, uint32_t divisor);
static void     selectUartPins(void);
static void     selectPinForTx(void);
static void     selectPinForRx(void);
static uint32_t calculatePinSelectionValue(uint32_t originalPinSelectionValue, uint32_t mask);
static void     enableUartToInterruptOnReceivedChar(void);
static void     configureNVICForUartInterrupt(void);
static int      calculateUartIndex(void);
void __mriLpc176xUart_Init(Token* pParameterTokens)
{
    UartParameters parameters;

    parseUartParameters(pParameterTokens, &parameters);
    saveUartToBeUsedByDebugger(parameters.uartIndex);
    if (parameters.share)
        setUartSharedFlag();
    else
        configureUartForExclusiveUseOfDebugger(&parameters);
}

static void parseUartParameters(Token* pParameterTokens, UartParameters* pParameters)
{
    static const char baudRatePrefix[] = "MRI_UART_BAUD=";
    const char*       pMatchingPrefix = NULL;
    
    memset(pParameters, 0, sizeof(*pParameters));

    if (Token_MatchingString(pParameterTokens, "MRI_UART_MBED_USB"))
        pParameters->uartIndex = 0;
    if (Token_MatchingString(pParameterTokens, "MRI_UART_MBED_P9_P10"))
        pParameters->uartIndex = 3;
    if (Token_MatchingString(pParameterTokens, "MRI_UART_MBED_P13_P14"))
        pParameters->uartIndex = 1;
    if (Token_MatchingString(pParameterTokens, "MRI_UART_MBED_P28_P27"))
        pParameters->uartIndex = 2;
        
    if ((pMatchingPrefix = Token_MatchingStringPrefix(pParameterTokens, baudRatePrefix)) != NULL)
        pParameters->baudRate = uint32FromString(pMatchingPrefix + sizeof(baudRatePrefix)-1);
    
    if (Token_MatchingString(pParameterTokens, "MRI_UART_SHARE"))
        pParameters->share = 1;
}

static void saveUartToBeUsedByDebugger(uint32_t mriUart)
{
    __mriLpc176xState.pCurrentUart = &g_uartConfigurations[mriUart];
}

static void setUartSharedFlag(void)
{
    __mriLpc176xState.flags |= LPC176X_UART_FLAGS_SHARE;
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
    Platform_CommPrepareToWaitForGdbConnection();
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

static void setUartBaudRate(UartParameters* pParameters)
{
    BaudRateDivisors  divisors;
    
    if (pParameters->baudRate == 0)
        return;
        
    divisors = calculateBaudRateDivisors(pParameters->baudRate, SystemCoreClock);
    setDivisors(&divisors);
    setManualBaudFlag();
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
    pThis->targetBaudRate = baudRate;
    pThis->adjustedPeripheralRate = fixupPeripheralRateFor16XOversampling(peripheralRate);
    pThis->mul = 1;
    pThis->divAdd = 0;
    pThis->closestDelta = ~0U;
    pThis->closestDivisor = pThis->adjustedPeripheralRate / pThis->targetBaudRate;
    pThis->closestMul = 1;
    pThis->closestDivAdd = 0;
}

static uint32_t fixupPeripheralRateFor16XOversampling(uint32_t actualPeripheralRate)
{
    return actualPeripheralRate >> 4;
}

static int isNoFractionalDivisorRequired(CalculateDivisors* pThis)
{
    return pThis->closestDivisor * pThis->targetBaudRate == pThis->adjustedPeripheralRate;
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
    uint32_t fixedTargetBaud = pThis->targetBaudRate << 4;
    uint32_t fixedResultingBaud;
    uint32_t fixedDelta;
    uint32_t divisor;

    divisor = calculateIntegerDivisorForTheseFractionalDivisors(pThis);
    fixedResultingBaud = calculateBaudRate(pThis, divisor);
    fixedDelta = (uint32_t)abs((int32_t)fixedResultingBaud - (int32_t)fixedTargetBaud);
    if (fixedDelta < pThis->closestDelta)
    {
        pThis->closestDelta = fixedDelta;
        pThis->closestDivisor = divisor;
        pThis->closestMul = pThis->mul;
        pThis->closestDivAdd = pThis->divAdd;
    }
}

static uint32_t calculateIntegerDivisorForTheseFractionalDivisors(CalculateDivisors* pThis)
{
    uint32_t scaledBaudRate = pThis->targetBaudRate;
    uint32_t fixedMul = pThis->mul << 4;
    uint32_t fixedAdjustedPeripheralRate = pThis->adjustedPeripheralRate << 4;
    uint32_t fixedTemp;
    
    /* Largest scaledBaudRate is < 4Mbit < 2^23
       Largest divAdd is 13 < 2^4
       Largest product is therefore 2^27 which is safe. */
    fixedTemp = scaledBaudRate * pThis->divAdd;

    /* Largest fixedTemp will be 2^27 which shifted left by 4 will be 2^31 
       Smallest fixedMul will be 2^1 * 2^4 = 2^5.
       Largest quotient is therefore 2^26. */
    fixedTemp = (fixedTemp << 4) / fixedMul;

    /* Adding a 2^26 + 2^23 which fits in 32-bit value. */
    fixedTemp += scaledBaudRate;

    /* Largest fixedAdjustedPeripheralRate is 120MHz/16 < 2^23 * 2^4 = 2^27
            when shifted left by another 4 will 2^31. */
    fixedTemp = (fixedAdjustedPeripheralRate << 4) / fixedTemp;
    
    /* Remove *16 multiplication caused by using scaled baud rate to increase its range, round the result as
       converting to integer from fixed point format. */
    return (fixedTemp + (1 << 7)) >> 8;
}

static uint32_t calculateBaudRate(CalculateDivisors* pThis, uint32_t divisor)
{
    uint32_t fixedDivisor = divisor << 4;
    uint32_t fixedMul = pThis->mul << 4;
    uint32_t fixedAdjustedPeripheralRate = pThis->adjustedPeripheralRate << 4;
    uint32_t fixedTemp;

    /* Largest fixedDivisor is for 300 baud at 128MHz < ((2^27 / 2^4) / 2^8) * 2^4 = (2^23 / 2^8) * 2^4 = 2^19
       Largest divAdd is 13 < 2^4
       Largest product is therefore 2^23 which is safe. */
    fixedTemp = fixedDivisor * pThis->divAdd;
    
    /* Largest fixedTemp will be 2^23 which shifted left by 4 will be 2^27
       Smallest fixedMul will be 2^1 * 2^4 = 2^5
       Largest quotient is therefore 2^22 */
    fixedTemp = (fixedTemp << 4) / fixedMul;
    
    /* Largest addend is 2^22. */
    fixedTemp += fixedDivisor;
    
    /* Largest fixedAdjustedPeripheralRate is 120MHz/16 < 2^23 * 2^4 = 2^27
            when shifted left by another 4 will be 2^31. */
    fixedTemp = (fixedAdjustedPeripheralRate << 4) / fixedTemp;
    
    return fixedTemp;
}

static void setDivisors(BaudRateDivisors* pDivisors)
{
    LPC_UART_TypeDef* pUartRegisters = __mriLpc176xState.pCurrentUart->pUartRegisters;

    setDivisorLatchBit();

    pUartRegisters->DLL = pDivisors->integerBaudRateDivisor & 0xFF;
    pUartRegisters->DLM = pDivisors->integerBaudRateDivisor >> 8;
    pUartRegisters->FDR = pDivisors->fractionalBaudRateDivisor;
    
    clearDivisorLatchBit();
}

#define LPC176x_UART_LCR_DLAB (1 << 7)

static void setDivisorLatchBit(void)
{
    __mriLpc176xState.pCurrentUart->pUartRegisters->LCR |= LPC176x_UART_LCR_DLAB;
}

static void clearDivisorLatchBit(void)
{
    __mriLpc176xState.pCurrentUart->pUartRegisters->LCR &= ~LPC176x_UART_LCR_DLAB;
}

static void setManualBaudFlag(void)
{
    __mriLpc176xState.flags |= LPC176X_UART_FLAGS_MANUAL_BAUD;
    
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


int Platform_CommSharingWithApplication(void)
{
    return __mriLpc176xState.flags & LPC176X_UART_FLAGS_SHARE;
}

static int isManualBaudRate(void);
int Platform_CommShouldWaitForGdbConnect(void)
{
    return !isManualBaudRate() && !Platform_CommSharingWithApplication();
}

static int isManualBaudRate(void)
{
    return (int)(__mriLpc176xState.flags & LPC176X_UART_FLAGS_MANUAL_BAUD);
}


int Platform_CommIsWaitingForGdbToConnect(void)
{
    static const uint32_t autoBaudStarting = 1;
    
    if (!Platform_CommShouldWaitForGdbConnect())
        return 0;

    return (int)(__mriLpc176xState.pCurrentUart->pUartRegisters->ACR & autoBaudStarting);
}


void Platform_CommPrepareToWaitForGdbConnection(void)
{
    static const uint32_t   autoBaudStart = 1;
    static const uint32_t   autoBaudModeForStartBitOnly = 1 << 1;
    static const uint32_t   autoBaudAutoRestart = 1 << 2;
    static const uint32_t   autoBaudValue = autoBaudStart | autoBaudModeForStartBitOnly | autoBaudAutoRestart;
    
    if (!Platform_CommShouldWaitForGdbConnect())
        return;
    
    __mriLpc176xState.pCurrentUart->pUartRegisters->ACR = autoBaudValue;
}


static int hasHostSentDataInLessThan10Milliseconds(void);
static int isNoReceivedCharAndNoTimeout(void);
void Platform_CommWaitForReceiveDataToStop(void)
{
    while (hasHostSentDataInLessThan10Milliseconds())
    {
        Platform_CommReceiveChar();
    }
}

static int hasHostSentDataInLessThan10Milliseconds(void)
{
    uint32_t originalSysTickControlValue = getCurrentSysTickControlValue();
    uint32_t originalSysTickReloadValue = getCurrentSysTickReloadValue();
    start10MillisecondSysTick();

    while (isNoReceivedCharAndNoTimeout())
    {
    }

    setSysTickReloadValue(originalSysTickReloadValue);
    setSysTickControlValue(originalSysTickControlValue);
    
    return Platform_CommHasReceiveData();
}

static int isNoReceivedCharAndNoTimeout(void)
{
    return !Platform_CommHasReceiveData() && !has10MillisecondSysTickExpired();
}
