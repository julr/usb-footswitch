#include "Board.h"

#include "stm32f4xx_ll_gpio.h"
#include "stm32f4xx_ll_bus.h"
#include "stm32f4xx_ll_rcc.h"
#include "stm32f4xx_ll_pwr.h"
#include "stm32f4xx_ll_system.h"

#include "tusb.h"

#include "ConfigManager.h"

// Class member
volatile std::uint32_t Board::tickCount_ {0U};
char Board::serialNumber_[14]{ '0' };
Board::ButtonState Board::buttonStates_[MAX_BUTTON_COUNT];

std::uint32_t Board::buttonCount_{0};

// USB ISR
extern "C" void OTG_FS_IRQHandler(void)
{
    tud_int_handler(0);
}

// SysTick ISR
extern "C" void SysTick_Handler(void)
{
    Board::tickCount_++;
    Board::debounceButtons();
}

//--------------------------------------------------------------------+
// MACRO TYPEDEF CONSTANT ENUM
//--------------------------------------------------------------------+

// Onboard LED
static GPIO_TypeDef* ONBOARDLED_PORT{GPIOC};
static constexpr std::uint32_t ONBOARD_LED_PIN{LL_GPIO_PIN_13};

// Onboard Button
static GPIO_TypeDef* ONBOARD_BUTTON_PORT{GPIOA};
static constexpr std::uint32_t ONBOARD_BUTTON_PIN{LL_GPIO_PIN_0};

// Foot buttons
static GPIO_TypeDef* FOOT_BUTTONS_PORT{GPIOB};
static constexpr std::uint32_t FOOT_BUTTON_1_PIN{LL_GPIO_PIN_9};
static constexpr std::uint32_t FOOT_BUTTON_2_PIN{LL_GPIO_PIN_7};
static constexpr std::uint32_t FOOT_BUTTON_3_PIN{LL_GPIO_PIN_5};
static constexpr std::uint32_t FOOT_BUTTON_4_PIN{LL_GPIO_PIN_3};

// Foot button leds
static GPIO_TypeDef* FOOT_LED_1_PORT{GPIOB};
static constexpr std::uint32_t FOOT_LED_1_PIN{LL_GPIO_PIN_8};
static GPIO_TypeDef* FOOT_LED_2_PORT{GPIOB};
static constexpr std::uint32_t FOOT_LED_2_PIN{LL_GPIO_PIN_6};
static GPIO_TypeDef* FOOT_LED_3_PORT{GPIOB};
static constexpr std::uint32_t FOOT_LED_3_PIN{LL_GPIO_PIN_4};
static GPIO_TypeDef* FOOT_LED_4_PORT{GPIOA};
static constexpr std::uint32_t FOOT_LED_4_PIN{LL_GPIO_PIN_15};

// Board config
static GPIO_TypeDef* BOARD_CONFIG_PORT{GPIOB};
static constexpr std::uint32_t BOARD_CONFIG_1_PIN{LL_GPIO_PIN_2};
static constexpr std::uint32_t BOARD_CONFIG_2_PIN{LL_GPIO_PIN_10};

void Board::initialize()
{
    initializeClock();
    SystemCoreClockUpdate();
    initializePins();
    readChipIdentifiaction();
    readBoardConfig();
    
    // the board doesn't use VBUS sense (B device) so disable it
    USB_OTG_FS->GCCFG |= USB_OTG_GCCFG_NOVBUSSENS;
    USB_OTG_FS->GCCFG &= ~(USB_OTG_GCCFG_VBUSBSEN | USB_OTG_GCCFG_VBUSASEN);
    
    // setup SysTick as 1ms tick timer
    SysTick_Config(SystemCoreClock / 1000);
}

void Board::setLed(LED led, bool on) //LED is low active
{
    GPIO_TypeDef* port;
    std::uint32_t pin;
    
    switch (led)
    {
    case LED::ONBOARD:
        if (on)
        {
            LL_GPIO_ResetOutputPin(ONBOARDLED_PORT, ONBOARD_LED_PIN);
        }
        else
        {
            LL_GPIO_SetOutputPin(ONBOARDLED_PORT, ONBOARD_LED_PIN);
        }
        return;
        
    case LED::FOOT_1:
        port = FOOT_LED_1_PORT;
        pin = FOOT_LED_1_PIN;
        break;
        
    case LED::FOOT_2:
        port = FOOT_LED_2_PORT;
        pin = FOOT_LED_2_PIN;
        break;
        
    case LED::FOOT_3:
        port = FOOT_LED_3_PORT;
        pin = FOOT_LED_3_PIN;
        break;
        
    case LED::FOOT_4:
        port = FOOT_LED_4_PORT;
        pin = FOOT_LED_4_PIN;
        break;
        
    default:
        return;
    }
    
    LL_GPIO_SetPinMode(port, pin, on ? LL_GPIO_MODE_OUTPUT : LL_GPIO_MODE_INPUT);
}

void Board::initializeClock()
{   
    // Power Control Clock
    LL_AHB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);
    
    // Set flash latency
    LL_FLASH_SetLatency(LL_FLASH_LATENCY_2);
    
    // The voltage scaling allows optimizing the power consumption when the device is
    // clocked below the maximum system frequency, to update the voltage scaling value
    // regarding system frequency refer to product datasheet.
    LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE2);
    
    // Enable HSE Oscillator and activate PLL with HSE as source
    // Main Clock will be set to 84 Mhz
    LL_RCC_HSE_Enable();
    while (LL_RCC_HSE_IsReady() != 1) ;
    
    constexpr std::uint32_t PLLM {HSE_VALUE / 1000000};
    constexpr std::uint32_t PLLN {336};
    LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSE, PLLM, PLLN, LL_RCC_PLLP_DIV_4);
    LL_RCC_PLL_ConfigDomain_48M(LL_RCC_PLLSOURCE_HSE, PLLM, PLLN, LL_RCC_PLLQ_DIV_7);
    LL_RCC_PLL_Enable();
    while (LL_RCC_PLL_IsReady() != 1) ;
    
    // Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
    // clocks dividers
    LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
    LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_2);
    LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);
    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
    while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL) ;
    
    // GPIO Clocks
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC);
    
    // USB Clock
    LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_OTGFS);
}


void Board::initializePins()
{
    // Onboard LED
    LL_GPIO_InitTypeDef LedInitStruct 
    {
        .Pin = ONBOARD_LED_PIN,
        .Mode = LL_GPIO_MODE_OUTPUT,
        .Speed = LL_GPIO_SPEED_FREQ_LOW,
        .OutputType = LL_GPIO_OUTPUT_PUSHPULL,
        .Pull = LL_GPIO_PULL_NO
    };
    LL_GPIO_Init(ONBOARDLED_PORT, &LedInitStruct);
    setLed(LED::ONBOARD, false);

    // Onboard Button
    LL_GPIO_InitTypeDef ButtonInitStruct 
    {
        .Pin = ONBOARD_BUTTON_PIN,
        .Mode = LL_GPIO_MODE_INPUT,
        .Speed = LL_GPIO_SPEED_FREQ_HIGH,
        .Pull = LL_GPIO_PULL_UP
    };
    LL_GPIO_Init(ONBOARD_BUTTON_PORT, &ButtonInitStruct);
   
    // USB FS GPIOs
    LL_GPIO_InitTypeDef UsbPinsInitStruct 
    {
        .Pin = LL_GPIO_PIN_11 | LL_GPIO_PIN_12,
        .Mode = LL_GPIO_MODE_ALTERNATE,
        .Speed = LL_GPIO_SPEED_FREQ_HIGH,
        .Pull = LL_GPIO_PULL_NO,
        .Alternate = LL_GPIO_AF_10
    };
    LL_GPIO_Init(GPIOA, &UsbPinsInitStruct);
    
    // VBUS Pin 
    LL_GPIO_InitTypeDef VbusPinInitStruct 
    {
        .Pin = LL_GPIO_PIN_9,
        .Mode = LL_GPIO_MODE_INPUT,
        .Speed = LL_GPIO_SPEED_FREQ_HIGH,
        .Pull = LL_GPIO_PULL_NO
    };
    LL_GPIO_Init(GPIOA, &VbusPinInitStruct);
    
    // ID Pin
    LL_GPIO_InitTypeDef IdPinInitStruct 
    {
        .Pin = LL_GPIO_PIN_10,
        .Mode = LL_GPIO_MODE_ALTERNATE,
        .Speed = LL_GPIO_SPEED_FREQ_HIGH,
        .Pull = LL_GPIO_PULL_UP,
        .Alternate = LL_GPIO_AF_10
    };
    LL_GPIO_Init(GPIOA, &IdPinInitStruct);
    
    // Foot Buttons
    LL_GPIO_InitTypeDef FootButtosInitStruct 
    {
        .Pin = FOOT_BUTTON_1_PIN | FOOT_BUTTON_2_PIN | FOOT_BUTTON_3_PIN | FOOT_BUTTON_4_PIN,
        .Mode = LL_GPIO_MODE_INPUT,
        .Speed = LL_GPIO_SPEED_FREQ_HIGH,
        .Pull = LL_GPIO_PULL_DOWN
    };
    LL_GPIO_Init(FOOT_BUTTONS_PORT, &FootButtosInitStruct);
    
    // Foot Leds
    LL_GPIO_InitTypeDef FootLedsInitStruct 
    {
        .Pin = FOOT_LED_1_PIN ,
        .Mode = LL_GPIO_MODE_INPUT,
        .Speed = LL_GPIO_SPEED_FREQ_HIGH,
        .Pull = LL_GPIO_PULL_NO
    };
    LL_GPIO_Init(FOOT_LED_1_PORT, &FootLedsInitStruct);
    LL_GPIO_ResetOutputPin(FOOT_LED_1_PORT, FOOT_LED_1_PIN);
    
    FootLedsInitStruct.Pin = FOOT_LED_2_PIN;
    LL_GPIO_Init(FOOT_LED_2_PORT, &FootLedsInitStruct);
    LL_GPIO_ResetOutputPin(FOOT_LED_2_PORT, FOOT_LED_2_PIN);
    
    FootLedsInitStruct.Pin = FOOT_LED_3_PIN;
    LL_GPIO_Init(FOOT_LED_3_PORT, &FootLedsInitStruct);
    LL_GPIO_ResetOutputPin(FOOT_LED_3_PORT, FOOT_LED_3_PIN);
    
    FootLedsInitStruct.Pin = FOOT_LED_4_PIN;
    LL_GPIO_Init(FOOT_LED_4_PORT, &FootLedsInitStruct);
    LL_GPIO_ResetOutputPin(FOOT_LED_4_PORT, FOOT_LED_4_PIN);
    
    // Board config
    LL_GPIO_InitTypeDef BoardConfigInitStruct 
    {
        .Pin = BOARD_CONFIG_1_PIN | BOARD_CONFIG_1_PIN,
        .Mode = LL_GPIO_MODE_INPUT,
        .Speed = LL_GPIO_SPEED_FREQ_HIGH,
        .Pull = LL_GPIO_PULL_NO
    };
    LL_GPIO_Init(BOARD_CONFIG_PORT, &BoardConfigInitStruct);
}


void Board::readChipIdentifiaction()
{
    // Read chip unique id and convert it to a string for usb serial number
    // The chip id is 96 bit wide, this reduces it to 32. Collisions might be possible
    // but accepted for this use case
    std::uint32_t* chip_id_ptr = reinterpret_cast<uint32_t*>(0x1FFF7A10);
    std::uint32_t chip_id_reduced = *chip_id_ptr ^ *(chip_id_ptr + 1) ^ *(chip_id_ptr + 2);
    // simple int to string conversion for uint32
    if (chip_id_reduced != 0)
    {
        char buffer[13];
        unsigned digits = 0;
        while (chip_id_reduced > 0)
        {
            buffer[digits++] = '0' + (chip_id_reduced % 10);
            chip_id_reduced /= 10;
        }
        serialNumber_[digits--] = '\0';
        //reverse
        for (unsigned i = 0; i <= digits; i++)
        {
            serialNumber_[digits - i] = buffer[i];
        }
    }
}


void Board::resetButtonStates()
{
    // Read the setup
    const Config& config{ConfigManager::getConfig()};
    
    const bool ledOn = config.SetLedMode == LedMode::ON;
     
    // Reset the button state and leds
    for (std::size_t i = 0; i < MAX_BUTTON_COUNT; i++)
    {
        buttonStates_[i].LastState = false;
        buttonStates_[i].ValidState = false;
        buttonStates_[i].DebounceCount = config.DebounceTime;
        setLed(static_cast<LED>(static_cast<std::size_t>(LED::FOOT_1) + i), ledOn);
    }
}


void Board::debounceButtons()
{   
    const std::uint32_t footButtonPort = LL_GPIO_ReadInputPort(FOOT_BUTTONS_PORT);
    const bool currentButtonStates[MAX_BUTTON_COUNT]
    {
        !LL_GPIO_IsInputPinSet(ONBOARD_BUTTON_PORT, ONBOARD_BUTTON_PIN), // Onboard button is low active
        ((footButtonPort & FOOT_BUTTON_1_PIN) != 0), // TODO Check polarity
        ((footButtonPort & FOOT_BUTTON_2_PIN) != 0),
        ((footButtonPort & FOOT_BUTTON_3_PIN) != 0),
        ((footButtonPort & FOOT_BUTTON_4_PIN) != 0),
    };
    
    const Config& config{ConfigManager::getConfig()};
    for (std::size_t i = 0; i < buttonCount_ + 1; i++) // onboard + external buttons
    {
        // state changed and debouncing is not deactivates
        if ((buttonStates_[i].LastState != currentButtonStates[i]) && (config.DebounceTime != 0))
        {
            buttonStates_[i].DebounceCount = config.DebounceTime;
        }
        else if(buttonStates_[i].DebounceCount > 0) // debouncing phase
        {
            buttonStates_[i].DebounceCount--;
        }
        else // Stable or not debouncing
        {
            buttonStates_[i].ValidState = currentButtonStates[i];
            
            // Leds only for foot buttons
            if (i > 0)
            {
                const LED led = static_cast<LED>(static_cast<std::size_t>(LED::FOOT_1) + i - 1);
                switch (config.SetLedMode)
                {
                case LedMode::NORMAL:
                    setLed(led, currentButtonStates[i]);
                    break;
                case LedMode::INVERTED:
                    setLed(led, !currentButtonStates[i]);
                    break;
                default:
                    break;
                }
            }
        }
        buttonStates_[i].LastState = currentButtonStates[i];
    }
}


void Board::readBoardConfig()
{
    buttonCount_ = 1; // at least one button is there
    
    LL_GPIO_SetPinPull(BOARD_CONFIG_PORT, BOARD_CONFIG_1_PIN, LL_GPIO_PULL_DOWN);
    LL_GPIO_SetPinPull(BOARD_CONFIG_PORT, BOARD_CONFIG_2_PIN, LL_GPIO_PULL_DOWN);
    
    if (LL_GPIO_IsInputPinSet(BOARD_CONFIG_PORT, BOARD_CONFIG_1_PIN))
        buttonCount_ +=1;
    if (LL_GPIO_IsInputPinSet(BOARD_CONFIG_PORT, BOARD_CONFIG_2_PIN))
        buttonCount_ += 2;
    
    LL_GPIO_SetPinPull(BOARD_CONFIG_PORT, BOARD_CONFIG_1_PIN, LL_GPIO_PULL_NO);
    LL_GPIO_SetPinPull(BOARD_CONFIG_PORT, BOARD_CONFIG_2_PIN, LL_GPIO_PULL_NO);
}


void Board::startBootloader()
{
    using FunctionPtr = void(*)();
    constexpr std::uint32_t* baseAddress{0};

    // Reset clocks
    LL_RCC_DeInit();
    
    // Disable SysTick
    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL = 0;
    
    // Remap system flash
    __DSB();
    LL_SYSCFG_SetRemapMemory(LL_SYSCFG_REMAP_SYSTEMFLASH);
    __DSB();
    __ISB();

    // Set user application SP
    __set_MSP(*baseAddress);
    
    // Get the bootloader function pointer
    FunctionPtr bootloaderFunction = reinterpret_cast<FunctionPtr>(*(baseAddress + 1));
    
    // Start bootloader
    bootloaderFunction();
    
    // Should never be executed, just in case of an error the system does not stall
    __NVIC_SystemReset();
}

// The Hal LL does not provide and flash drivers, this needs to be implemented by hand
bool Board::eraseUserFlash()
{
    if (!unlockFlash()) return false;
    
    waitForFlashReady();
    
    FLASH->CR &= ~FLASH_CR_PSIZE_Msk; // Clear program parallels
    FLASH->CR |= FLASH_CR_PSIZE_1; // Set program parallelism to x32
    FLASH->CR |= FLASH_CR_SER; // Sector Erase
    FLASH->CR &= ~FLASH_CR_SNB_Msk; // Clear selected sector
    FLASH->CR |= FLASH_CR_SNB_0 | FLASH_CR_SNB_2; // Select sector 5
    FLASH->CR |= FLASH_CR_STRT; // Start action
    
    waitForFlashReady();
    
    FLASH->CR &= ~FLASH_CR_SER; // Clear sector erase
    lockFlash();
    
    return (FLASH->SR & (FLASH_SR_RDERR | FLASH_SR_PGSERR | FLASH_SR_PGPERR | FLASH_SR_PGAERR | FLASH_SR_WRPERR)) == 0;
}


bool Board::writeUserFlash(std::uint32_t address, void* data, std::size_t length)
{
    if ((address + length) > (USER_FLASH_SECTION_START + USER_FLASH_SECTION_SIZE)) return false;
    if (!unlockFlash()) return false;
    waitForFlashReady();
    
    FLASH->CR &= ~FLASH_CR_PSIZE_Msk; // Clear program parallels -> x8
    for (std::size_t i = 0; i < length; i++)
    {
        FLASH->CR |= FLASH_CR_PG; // Enable program mode
        *(reinterpret_cast<std::uint8_t*>(address + i)) = *(static_cast<std::uint8_t*>(data) + i);
        waitForFlashReady();
    }
    
    FLASH->CR &= ~FLASH_CR_PG; // Clear program mode
    lockFlash();
    
    return (FLASH->SR & (FLASH_SR_RDERR | FLASH_SR_PGSERR | FLASH_SR_PGPERR | FLASH_SR_PGAERR | FLASH_SR_WRPERR)) == 0;
}


bool Board::unlockFlash()
{
    if ((FLASH->CR & FLASH_CR_LOCK) == 0) return true; // already unlocked
    FLASH->KEYR = FLASH_KEY1;
    FLASH->KEYR = FLASH_KEY2;    
    return (FLASH->CR & FLASH_CR_LOCK) == 0;
}


void Board::lockFlash()
{
    FLASH->CR |= FLASH_CR_LOCK;
}


void Board::waitForFlashReady()
{
    while (FLASH->SR & FLASH_SR_BSY);
}
