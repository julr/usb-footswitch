#pragma once

#include <cstdint>

extern "C" void SysTick_Handler();

enum class Button
{
    ONBOARD,
    FOOT_1,
    FOOT_2,
    FOOT_3,
    FOOT_4
};

enum class LED
{
    ONBOARD,
    FOOT_1,
    FOOT_2,
    FOOT_3,
    FOOT_4
};

class Board
{
public:
    static constexpr std::uint32_t USER_FLASH_SECTION_START{0x08020000}; // Flash Sector 5 start address
    static constexpr std::size_t USER_FLASH_SECTION_SIZE{128 * 1024}; // Flash Sector 5 size
    
    friend void(::SysTick_Handler)();
    
    static void initialize();
    
    static void setLed(LED led, bool on);
    
    static bool getButtonState(Button button)
    {
        return buttonStates_[static_cast<std::size_t>(button)].ValidState;
    }
    
    static void resetButtonStates();
    
    static char* getSerialNumber()
    {
        return serialNumber_;
    }
    
    static std::uint32_t getNumberOfButtons()
    {
        return buttonCount_;
    }
    
    static std::uint32_t getMillis()
    {
        return tickCount_;
    }
    
    static void startBootloader();
    
    static bool eraseUserFlash();
    
    static bool writeUserFlash(std::uint32_t address, void* data, std::size_t length);

private:
    static void initializeClock();
    
    static void initializePins();
    
    static void readChipIdentifiaction();
    
    static void readBoardConfig();
    
    static void debounceButtons();
    
    static bool unlockFlash();
    
    static void lockFlash();
    static void waitForFlashReady();

    
    struct ButtonState
    {
        bool ValidState;
        bool LastState;
        std::uint8_t DebounceCount;
    };
    
    // Foot buttons + onboard button
    static constexpr std::size_t MAX_BUTTON_COUNT{5};
    
    static constexpr std::uint32_t FLASH_KEY1{0x45670123};
    static constexpr std::uint32_t FLASH_KEY2{0xCDEF89AB};
    
    static ButtonState buttonStates_[MAX_BUTTON_COUNT];
    static volatile std::uint32_t tickCount_;
    static char serialNumber_[14];
    
    static uint32_t buttonCount_;
};    