/* Copyright 2022 Adam Green     (https://github.com/adamgreen/)
   Copyright 2015 Chang,Jia-Rung (https://github.com/JaredCJR)

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
/* Routines used to provide STM32F429xx USART functionality to the mri debugger. */
#include <stdlib.h>
#include <core/libc.h>
#include <core/platforms.h>
#include <architectures/armv7-m/debug_cm3.h>
#include "stm32f411xx_init.h"
#include "stm32f411xx_usart.h"
#include <architectures/armv7-m/armv7-m.h>

/* Start indices at 0 such that UART1 is at index 0, UART2 is at index 1, etc. */
static const UartConfiguration g_uartConfigurations[] =
{
    {
        /*
         * Tx=PA9
         * Rx=PA10
         */
        USART1,
        7, /* AF7 */
        7  /* AF7 */
    },
    {
        /*
         * Tx=PA2
         * Rx=PA3
         */
        USART2,
        7, /* AF7 */
        7  /* AF7 */
    }
};


typedef struct
{
    uint32_t uartIndex;
    uint32_t baudRate;
} UartParameters;


static void     configureNVICForUartInterrupt(uint32_t index);
static void     parseUartParameters(Token* pParameterTokens, UartParameters* pParameters);
static void     saveUartToBeUsedByDebugger(uint32_t mriUart);
static void     configureUartForExclusiveUseOfDebugger(UartParameters* pParameters);
static int      commUartIndex(void);

static uint32_t getDecimalDigit(char currChar)
{
    if (currChar >= '0' && currChar <= '9')
        return currChar - '0';
    else
        __throw_and_return(invalidDecDigitException, 0);
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

void mriStm32f411xxUart_Init(Token *pParameterTokens)
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

    if (Token_MatchingString(pParameterTokens, "MRI_UART_1"))
        pParameters->uartIndex = 1;
    if (Token_MatchingString(pParameterTokens, "MRI_UART_2"))
        pParameters->uartIndex = 2;

    if ((pMatchingPrefix = Token_MatchingStringPrefix(pParameterTokens, baudRatePrefix)) != NULL)
    {
        pParameters->baudRate = uint32FromString(pMatchingPrefix + sizeof(baudRatePrefix)-1);
    }
    else
    {
        /* Default baud rate to 230400. */
        pParameters->baudRate = 230400;
    }
}



static void saveUartToBeUsedByDebugger(uint32_t mriUart)
{
    /* -1 is due to the array start by index 0.However,we start from UART"1". */
    mriStm32f411xxState.pCurrentUart = &g_uartConfigurations[mriUart-1];
}



static void enableUartPeripheralCLOCK(uint32_t uart)
{
    /*
     * USART1:APB2ENR;  GPIOA:AHB1
     * USART2:APB1ENR;  GPIOA:AHB1
     */
    switch(uart)
    {
        case 1: /* USART1 */
            RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
            RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
            break;
        case 2: /* USART2 */
            RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
            RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
            break;
        default: /* USART1 */
            RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
            RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    }
}

typedef struct
{
    uint32_t _mriSYSCLK_Frequency; /*!<  SYSCLK clock frequency expressed in Hz */
    uint32_t _mriHCLK_Frequency;   /*!<  HCLK clock frequency expressed in Hz   */
    uint32_t _mriPCLK1_Frequency;  /*!<  PCLK1 clock frequency expressed in Hz  */
    uint32_t _mriPCLK2_Frequency;  /*!<  PCLK2 clock frequency expressed in Hz  */
} _mriRCC_ClocksTypeDef;

static __I uint8_t _mriAPBAHBPrescTable[16] = {0, 0, 0, 0, 1, 2, 3, 4, 1, 2, 3, 4, 6, 7, 8, 9};

#define HSI_VALUE ((uint32_t)16000000)
#define HSE_VALUE ((uint32_t)25000000)

static void _mriRCC_GetClocksFreq(_mriRCC_ClocksTypeDef* RCC_Clocks)
{
    uint32_t tmp = 0, presc = 0, pllvco = 0, pllp = 2, pllsource = 0, pllm = 2;

    /* Get SYSCLK source -------------------------------------------------------*/
    tmp = RCC->CFGR & RCC_CFGR_SWS;

    switch (tmp)
    {
        case 0x00:  /* HSI used as system clock source */
            RCC_Clocks->_mriSYSCLK_Frequency = HSI_VALUE;
            break;
        case 0x04:  /* HSE used as system clock  source */
            RCC_Clocks->_mriSYSCLK_Frequency = HSE_VALUE;
            break;
        case 0x08:  /* PLL used as system clock  source */

            /* PLL_VCO = (HSE_VALUE or HSI_VALUE / PLLM) * PLLN
               SYSCLK = PLL_VCO / PLLP
               */
            pllsource = (RCC->PLLCFGR & RCC_PLLCFGR_PLLSRC) >> 22;
            pllm = RCC->PLLCFGR & RCC_PLLCFGR_PLLM;

            if (pllsource != 0)
            {
                /* HSE used as PLL clock source */
                pllvco = (HSE_VALUE / pllm) * ((RCC->PLLCFGR & RCC_PLLCFGR_PLLN) >> 6);
            }
            else
            {
                /* HSI used as PLL clock source */
                pllvco = (HSI_VALUE / pllm) * ((RCC->PLLCFGR & RCC_PLLCFGR_PLLN) >> 6);
            }

            pllp = (((RCC->PLLCFGR & RCC_PLLCFGR_PLLP) >>16) + 1 ) *2;
            RCC_Clocks->_mriSYSCLK_Frequency = pllvco/pllp;
            break;
        default:
            RCC_Clocks->_mriSYSCLK_Frequency = HSI_VALUE;
            break;
    }
    /* Compute HCLK, PCLK1 and PCLK2 clocks frequencies ------------------------*/

    /* Get HCLK prescaler */
    tmp = RCC->CFGR & RCC_CFGR_HPRE;
    tmp = tmp >> 4;
    presc = _mriAPBAHBPrescTable[tmp];
    /* HCLK clock frequency */
    RCC_Clocks->_mriHCLK_Frequency = RCC_Clocks->_mriSYSCLK_Frequency >> presc;

    /* Get PCLK1 prescaler */
    tmp = RCC->CFGR & RCC_CFGR_PPRE1;
    tmp = tmp >> 10;
    presc = _mriAPBAHBPrescTable[tmp];
    /* PCLK1 clock frequency */
    RCC_Clocks->_mriPCLK1_Frequency = RCC_Clocks->_mriHCLK_Frequency >> presc;

    /* Get PCLK2 prescaler */
    tmp = RCC->CFGR & RCC_CFGR_PPRE2;
    tmp = tmp >> 13;
    presc = _mriAPBAHBPrescTable[tmp];
    /* PCLK2 clock frequency */
    RCC_Clocks->_mriPCLK2_Frequency = RCC_Clocks->_mriHCLK_Frequency >> presc;
}

/* Calculates the value for the USART_BRR */
static uint16_t usart_baud_calc(uint32_t base,USART_TypeDef *USARTx,uint32_t baudrate)
{
    uint32_t tmpreg = 0x00, apbclock = 0x00;
    uint32_t integerdivider = 0x00;
    uint32_t fractionaldivider = 0x00;
    _mriRCC_ClocksTypeDef RCC_ClocksStatus;

    /* Configure the USART Baud Rate */
    _mriRCC_GetClocksFreq(&RCC_ClocksStatus);

    if ((base == USART1_BASE) || (base == USART6_BASE))
    {
        apbclock = RCC_ClocksStatus._mriPCLK2_Frequency;
    }
    else
    {
        apbclock = RCC_ClocksStatus._mriPCLK1_Frequency;
    }

    /* Determine the integer part */
    if ((USARTx->CR1 & USART_CR1_OVER8) != 0)
    {
        /* Integer part computing in case Oversampling mode is 8 Samples */
        integerdivider = ((25 * apbclock) / (2 * (baudrate)));
    }
    else
    { /* if ((USARTx->CR1 & USART_CR1_OVER8) == 0) */
        /* Integer part computing in case Oversampling mode is 16 Samples */
        integerdivider = ((25 * apbclock) / (4 * (baudrate)));
    }
    tmpreg = (integerdivider / 100) << 4;

    /* Determine the fractional part */
    fractionaldivider = integerdivider - (100 * (tmpreg >> 4));

    /* Implement the fractional part in the register */
    if ((USARTx->CR1 & USART_CR1_OVER8) != 0)
    {
        tmpreg |= ((((fractionaldivider * 8) + 50) / 100)) & ((uint8_t)0x07);
    }
    else
    { /* if ((USARTx->CR1 & USART_CR1_OVER8) == 0) */
        tmpreg |= ((((fractionaldivider * 16) + 50) / 100)) & ((uint8_t)0x0F);
    }

    /* Write to USART BRR register */
    return (uint16_t)tmpreg;
}

static void enableUART(UartParameters *pParameters)
{
    uint32_t uart = pParameters->uartIndex;
    USART_TypeDef* _uart = g_uartConfigurations[uart-1].pUartRegisters;
    /*******************************___CR2___********************************/
    /*
     * 00 = 1 stop-bit
     */
    _uart->CR2 &= ~USART_CR2_STOP;

    /*******************************___CR1___********************************/
    /*
     * Word Length : 8 Data bits
     */
    _uart->CR1 &= ~USART_CR1_M;

    /*
     * Parity bit NO
     */
    _uart->CR1 &= ~USART_CR1_PCE;

    /*
     * USART Mode
     */
    _uart->CR1 |= ( USART_CR1_RE | USART_CR1_TE );

    /*******************************___CR3___********************************/
    /*
     * Flow Control don't need
     */
    _uart->CR3 &= ~USART_CR3_RTSE; /* disable RTS flow control */
    _uart->CR3 &= ~USART_CR3_CTSE; /* disable CTS flow control */
    /*******************************___BRR___********************************/
    /*
     * Set baud-rate
     */
    uint32_t base_addr = USART1_BASE;
    USART_TypeDef *USARTx = USART1;
    switch(uart)
    {
        case 1: /* USART1 */
            base_addr = USART1_BASE;
            USARTx = USART1;
            break;
        case 2: /* USART2 */
            base_addr = USART2_BASE;
            USARTx = USART2;
            break;
    }

    _uart->BRR = usart_baud_calc(base_addr,USARTx,pParameters->baudRate);

    /*
     * Enable USART
     */
    _uart->CR1 |= USART_CR1_UE;

}


#define GPIO_PUPDR_M(n)                 (uint32_t) (0x3 << (2*n))          /* Pin mask */
#define GPIO_PUPDR_PIN(n)               (uint32_t) (2*n)                   /* Pin bitshift */
#define GPIO_PUPDR_NONE                 (uint32_t) (0x0)                   /* Port no pull-up, pull-down */
#define GPIO_MODER_M(n)                 (uint32_t) (0x3 << 2*n)            /* Pin mask */
#define GPIO_MODER_PIN(n)               (uint32_t) (2*n)                   /* Pin bitshift */
#define GPIO_MODER_ALT                  (uint32_t) (0x2)                   /* Alternative function mode */
/* All GPIO(contains USARTs) on AHB1 */
static void enableGPIO(uint32_t uart)
{
    /* USART1:GPIO_A
     * USART2:GPIO_A
     */
    GPIO_TypeDef *my_GPIO;
    uint8_t pin_tx = 9;
    uint8_t pin_rx = 10;
    /* ToDo: Using better function */
    switch(uart)
    {
        case 1: /* USART1 */
            my_GPIO = GPIOA;
            pin_tx = 9;
            pin_rx = 10;
            break;
        case 2: /* USART2 */
            my_GPIO = GPIOA;
            pin_tx = 2;
            pin_rx = 3;
            break;
        default: /* USART1 */
            my_GPIO = GPIOA;
            pin_tx = 9;
            pin_rx = 10;
            break;
    }
    /*
     * Set to be non Push-pull
     */
    uint32_t mode = GPIO_PUPDR_NONE;
    my_GPIO->PUPDR &= ~(GPIO_PUPDR_M(pin_tx));
    my_GPIO->PUPDR |= (mode << GPIO_PUPDR_PIN(pin_tx));
    my_GPIO->PUPDR &= ~(GPIO_PUPDR_M(pin_rx));
    my_GPIO->PUPDR |= (mode << GPIO_PUPDR_PIN(pin_rx));

    /*
     * Mode type
     * Set to be alternative function
     */
    uint32_t type = GPIO_MODER_ALT;
    my_GPIO->MODER &= ~(GPIO_MODER_M(pin_tx));
    my_GPIO->MODER |= (type << GPIO_MODER_PIN(pin_tx));
    my_GPIO->MODER &= ~(GPIO_MODER_M(pin_rx));
    my_GPIO->MODER |= (type << GPIO_MODER_PIN(pin_rx));

    /*
     * For Alternative-Function,assign AF
     * USART1/2 are AF7
     */
    /* ToDo: Using better function */
    switch(uart)
    {
        case 2: /* USART2 */
            my_GPIO->AFR[0] &= ~0x00000F00; /* Pin 2,tx */
            my_GPIO->AFR[0] |= (g_uartConfigurations[1].txFunction << 2*4);
            my_GPIO->AFR[0] &= ~0x0000F000; /* Pin 3,rx */
            my_GPIO->AFR[0] |= (g_uartConfigurations[1].rxFunction << 3*4);
            break;
        case 1: /* USART1 */
        default:
            my_GPIO->AFR[1] &= ~0x000000F0; /* Pin 9,tx */
            my_GPIO->AFR[1] |= (g_uartConfigurations[0].txFunction << 1*4);
            my_GPIO->AFR[1] &= ~0x00000F00; /* Pin 10,rx */
            my_GPIO->AFR[1] |= (g_uartConfigurations[0].rxFunction << 2*4);
            break;
    }

    /*
     * GPIO output type
     */
#define GPIO_OTYPER_M(n)                (uint32_t) (1 << n)                  /* Pin mask */
#define GPIO_OTYPER_PIN(n)              (uint32_t) (n)                       /* Pin bitshift */
#define GPIO_OTYPER_OUTPUT_PUSHPULL     0                                    /* Push Pull */
    my_GPIO->OTYPER &= ~GPIO_OTYPER_M(pin_tx);
    my_GPIO->OTYPER |= (GPIO_OTYPER_OUTPUT_PUSHPULL << GPIO_OTYPER_PIN(pin_tx) );
    my_GPIO->OTYPER &= ~GPIO_OTYPER_M(pin_rx);
    my_GPIO->OTYPER |= (GPIO_OTYPER_OUTPUT_PUSHPULL << GPIO_OTYPER_PIN(pin_rx) );

    /*
     * GPIO speed
     */
#define GPIO_OSPEEDR_M(n)               (uint32_t) (0x3 << (2*n))           /* Pin mask */
#define GPIO_OSPEEDR_PIN(n)             (uint32_t) (2*n)                    /* Pin bitshift */
#define GPIO_OSPEEDR_50M                (uint32_t) (0x2)                    /* Output speed 50MHz */
    uint32_t speed = GPIO_OSPEEDR_50M;
    my_GPIO->OSPEEDR &= ~(GPIO_OSPEEDR_M(pin_tx));
    my_GPIO->OSPEEDR |= (speed << GPIO_OSPEEDR_PIN(pin_tx));
    my_GPIO->OSPEEDR &= ~(GPIO_OSPEEDR_M(pin_rx));
    my_GPIO->OSPEEDR |= (speed << GPIO_OSPEEDR_PIN(pin_rx));
}


static void enableUartToInterruptOnReceivedChar(uint32_t index)
{
    mriStm32f411xxState.pCurrentUart->pUartRegisters->CR1 |= USART_CR1_RXNEIE;
}


/* If the order of g_uartConfigurations changes,fix it!
    Return 0 for USART1
    Return 1 for USART2
    Return 2 for USART3
*/
static int commUartIndex(void)
{
    return mriStm32f411xxState.pCurrentUart - g_uartConfigurations;
}

uint32_t Platform_CommHasReceiveData(void)
{
    return mriStm32f411xxState.pCurrentUart->pUartRegisters->SR & USART_SR_RXNE;
}

uint32_t Platform_CommHasTransmitCompleted(void)
{
    return mriStm32f411xxState.pCurrentUart->pUartRegisters->SR & USART_SR_TC;
}

int Platform_CommReceiveChar(void)
{
    while(!Platform_CommHasReceiveData())
    {
        /* busy wait */
    }
    return (mriStm32f411xxState.pCurrentUart->pUartRegisters->DR & 0x1FF);
}



void Platform_CommSendChar(int Character)
{
    USART_TypeDef *uart = mriStm32f411xxState.pCurrentUart->pUartRegisters;
    while (!(uart->SR & USART_SR_TXE))
    {
        /* busy wait */
    }
    uart->DR = (Character & 0x1FF);
}

static void configureNVICForUartInterrupt(uint32_t index)
{
    IRQn_Type irq_num_base = USART1_IRQn;
    /*
     * For USART1~3,the IRQn are continuous,others need check!
     */
    IRQn_Type currentUartIRQ;
    currentUartIRQ = (IRQn_Type)((int)irq_num_base + commUartIndex()) ;
    mriCortexMSetPriority(currentUartIRQ, 0, 0);
    NVIC_EnableIRQ(currentUartIRQ);
}

static void configureUartForExclusiveUseOfDebugger(UartParameters* pParameters)
{
    uint32_t uart_index = pParameters->uartIndex;
    enableUartPeripheralCLOCK(uart_index);
    enableGPIO(uart_index);
    enableUART(pParameters);
    enableUartToInterruptOnReceivedChar(uart_index);
    configureNVICForUartInterrupt(uart_index);
}
