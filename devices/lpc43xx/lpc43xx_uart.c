/* Copyright 2015 Adam Green (http://mbed.org/users/AdamGreen/)

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
/* Routines used to provide LPC43xx UART functionality to the mri debugger. */
#include <string.h>
#include <stdlib.h>
#include "platforms.h"
#include "../../architectures/armv7-m/debug_cm3.h"
#include "lpc43xx_init.h"


static const UartConfiguration g_uartConfigurations[] =
{
    {
        LPC_USART0,
        CLK_BASE_UART0,
        CLK_MX_UART0,
        CLK_APB0_UART0,
        SCU_PIN(6, 4),
        SCU_MODE_FUNC2,
        SCU_PIN(6, 5),
        SCU_MODE_FUNC2
    },
    {
        LPC_UART1,
        CLK_BASE_UART1,
        CLK_MX_UART1,
        CLK_APB0_UART1,
        SCU_PIN(5, 6),
        SCU_MODE_FUNC4,
        SCU_PIN(1, 14),
        SCU_MODE_FUNC1
    },
    {
        LPC_USART2,
        CLK_BASE_UART2,
        CLK_MX_UART2,
        CLK_APB2_UART2,
        SCU_PIN(2, 10),
        SCU_MODE_FUNC2,
        SCU_PIN(2, 11),
        SCU_MODE_FUNC2
    },
    {
        LPC_USART3,
        CLK_BASE_UART3,
        CLK_MX_UART3,
        CLK_APB2_UART3,
        SCU_PIN(2, 3),
        SCU_MODE_FUNC2,
        SCU_PIN(2, 4),
        SCU_MODE_FUNC2
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
static void     setUartSharedFlag(void);
static uint32_t uint32FromString(const char* pString);
static uint32_t getDecimalDigit(char currChar);
static void     configureUartForExclusiveUseOfDebugger(UartParameters* pParameters);
static void     setUartPeripheralClockToPLL1(void);
static void     enableUartClocks(void);
static void     enableCCUClock(CCU_CLK_T clockToEnable);
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
static void     selectUartPins(void);
static void     enableUartToInterruptOnReceivedChar(void);
static void     configureNVICForUartInterrupt(void);
void __mriLpc43xxUart_Init(Token* pParameterTokens)
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

    if (Token_MatchingString(pParameterTokens, "MRI_UART_SHARE"))
        pParameters->share = 1;
}

static void saveUartToBeUsedByDebugger(uint32_t mriUart)
{
    __mriLpc43xxState.pCurrentUart = &g_uartConfigurations[mriUart];
}

static void setUartSharedFlag(void)
{
    __mriLpc43xxState.flags |= LPC43XX_UART_FLAGS_SHARE;
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
    setUartPeripheralClockToPLL1();
    enableUartClocks();
    clearUartFractionalBaudDivisor();
    enableUartFifoAndDisableDma();
    setUartTo8N1();
    setUartBaudRate(pParameters);
    selectUartPins();
    enableUartToInterruptOnReceivedChar();
    Platform_CommPrepareToWaitForGdbConnection();
    configureNVICForUartInterrupt();
}

static void setUartPeripheralClockToPLL1(void)
{
    static const uint32_t autoBlockBit = 1 << 11;
    static const uint32_t pll1Bit = 0x09 << 24;

    LPC_CGU->BASE_CLK[__mriLpc43xxState.pCurrentUart->baseClock] = autoBlockBit | pll1Bit;
}

static void enableUartClocks(void)
{
    enableCCUClock(__mriLpc43xxState.pCurrentUart->registerClock);
    enableCCUClock(__mriLpc43xxState.pCurrentUart->peripheralClock);
}

static void enableCCUClock(CCU_CLK_T clockToEnable)
{
    static const uint32_t runBit = 1 << 0;

    if (clockToEnable < CLK_CCU2_START)
        LPC_CCU1->CLKCCU[clockToEnable].CFG = runBit;
    else
        LPC_CCU2->CLKCCU[clockToEnable - CLK_CCU2_START].CFG = runBit;
}

static void clearUartFractionalBaudDivisor(void)
{
    __mriLpc43xxState.pCurrentUart->pUartRegisters->FDR = 0x10;
}

static void enableUartFifoAndDisableDma(void)
{
    static const uint32_t enableFifoDisableDmaSetReceiveInterruptThresholdTo0 = 0x01;

    __mriLpc43xxState.pCurrentUart->pUartRegisters->FCR = enableFifoDisableDmaSetReceiveInterruptThresholdTo0;
}

static void setUartTo8N1(void)
{
    static const uint8_t wordLength8Bit = 0x3;
    static const uint8_t stopBit1 = 0 << 2;
    static const uint8_t disableParity = 0 << 3;
    static const uint8_t lineControlValueFor8N1 = wordLength8Bit | disableParity | stopBit1;

    __mriLpc43xxState.pCurrentUart->pUartRegisters->LCR = lineControlValueFor8N1;
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
    fixedDelta = abs(fixedRatio - pThis->desiredRatio);

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
    LPC_USART_T* pUartRegisters = __mriLpc43xxState.pCurrentUart->pUartRegisters;

    setDivisorLatchBit();

    pUartRegisters->DLL = pDivisors->integerBaudRateDivisor & 0xFF;
    pUartRegisters->DLM = pDivisors->integerBaudRateDivisor >> 8;
    pUartRegisters->FDR = pDivisors->fractionalBaudRateDivisor;

    clearDivisorLatchBit();
}

#define LPC176x_UART_LCR_DLAB (1 << 7)

static void setDivisorLatchBit(void)
{
    __mriLpc43xxState.pCurrentUart->pUartRegisters->LCR |= LPC176x_UART_LCR_DLAB;
}

static void clearDivisorLatchBit(void)
{
    __mriLpc43xxState.pCurrentUart->pUartRegisters->LCR &= ~LPC176x_UART_LCR_DLAB;
}

static void setManualBaudFlag(void)
{
    __mriLpc43xxState.flags |= LPC43XX_UART_FLAGS_MANUAL_BAUD;

}

static void selectUartPins(void)
{
    const UartConfiguration*  pUart = __mriLpc43xxState.pCurrentUart;
    uint32_t                  txPin = pUart->txPin;
    uint32_t                  rxPin = pUart->rxPin;

    LPC_SCU->SFSP[txPin >> 16][txPin & 0xFFFF] = pUart->txFunction;
    LPC_SCU->SFSP[rxPin >> 16][rxPin & 0xFFFF] = pUart->rxFunction | SCU_PINIO_PULLNONE;
}

static void enableUartToInterruptOnReceivedChar(void)
{
    static const uint32_t baudDivisorLatchBit = (1 << 7);
    static const uint32_t enableReceiveDataInterrupt = (1 << 0);
    uint32_t              originalLCR;

    originalLCR = __mriLpc43xxState.pCurrentUart->pUartRegisters->LCR;
    __mriLpc43xxState.pCurrentUart->pUartRegisters->LCR &= ~baudDivisorLatchBit;
    __mriLpc43xxState.pCurrentUart->pUartRegisters->IER = enableReceiveDataInterrupt;
    __mriLpc43xxState.pCurrentUart->pUartRegisters->LCR = originalLCR;
}

static void configureNVICForUartInterrupt(void)
{
    IRQn_Type uart0BaseIRQ = USART0_IRQn;
    IRQn_Type currentUartIRQ;

    currentUartIRQ = (IRQn_Type)((int)uart0BaseIRQ + Platform_CommUartIndex());
    NVIC_SetPriority(currentUartIRQ, 0);
    NVIC_EnableIRQ(currentUartIRQ);
}


int Platform_CommUartIndex(void)
{
    return __mriLpc43xxState.pCurrentUart - g_uartConfigurations;
}


uint32_t Platform_CommHasReceiveData(void)
{
    static const uint8_t receiverDataReadyBit = 1 << 0;

    return __mriLpc43xxState.pCurrentUart->pUartRegisters->LSR & receiverDataReadyBit;
}


static void     waitForUartToReceiveData(void);
int Platform_CommReceiveChar(void)
{
    waitForUartToReceiveData();

    return (int)__mriLpc43xxState.pCurrentUart->pUartRegisters->RBR;
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

    __mriLpc43xxState.pCurrentUart->pUartRegisters->THR = (uint8_t)Character;
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

    return __mriLpc43xxState.pCurrentUart->pUartRegisters->LSR & transmitterHoldRegisterEmptyBit;
}


int Platform_CommCausedInterrupt(void)
{
    const uint32_t uart0BaseExceptionId = USART0_IRQn + 16;
    uint32_t       interruptSource = getCurrentlyExecutingExceptionNumber();
    uint32_t       currentUartExceptionId;

    currentUartExceptionId = uart0BaseExceptionId + Platform_CommUartIndex();
    return interruptSource == currentUartExceptionId;
}


void Platform_CommClearInterrupt(void)
{
    uint32_t interruptId;

    interruptId = __mriLpc43xxState.pCurrentUart->pUartRegisters->IIR;
    (void)interruptId;
}


int Platform_CommSharingWithApplication(void)
{
    return __mriLpc43xxState.flags & LPC43XX_UART_FLAGS_SHARE;
}

static int isManualBaudRate(void);
int Platform_CommShouldWaitForGdbConnect(void)
{
    return !isManualBaudRate() && !Platform_CommSharingWithApplication();
}

static int isManualBaudRate(void)
{
    return (int)(__mriLpc43xxState.flags & LPC43XX_UART_FLAGS_MANUAL_BAUD);
}


int Platform_CommIsWaitingForGdbToConnect(void)
{
    static const uint32_t autoBaudStarting = 1;

    if (!Platform_CommShouldWaitForGdbConnect())
        return 0;

    return (int)(__mriLpc43xxState.pCurrentUart->pUartRegisters->ACR & autoBaudStarting);
}


void Platform_CommPrepareToWaitForGdbConnection(void)
{
    static const uint32_t   autoBaudStart = 1;
    static const uint32_t   autoBaudModeForStartBitOnly = 1 << 1;
    static const uint32_t   autoBaudAutoRestart = 1 << 2;
    static const uint32_t   autoBaudValue = autoBaudStart | autoBaudModeForStartBitOnly | autoBaudAutoRestart;

    if (!Platform_CommShouldWaitForGdbConnect())
        return;

    __mriLpc43xxState.pCurrentUart->pUartRegisters->ACR = autoBaudValue;
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
