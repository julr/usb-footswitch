#pragma once
#include "Config.h"
#include "Board.h"

class ConfigManager
{
    
public:
    static void initialize();    
    
    static bool setButtonConfig(std::size_t button, std::uint8_t scanCode, std::uint8_t modifier);
    
    static void setButtonDebounceTime(std::uint8_t debounceTime);
    
    static bool setLedMode(LedMode ledMode);
    
    static bool saveConfig();
    
    static const Config& getConfig()
    {
        return currentConfig_;
    }
private:
    static constexpr std::size_t CONFIG_SIZE{sizeof(Config)};
    
    static constexpr std::size_t LAST_CONFIG_INDEX{(Board::USER_FLASH_SECTION_SIZE / CONFIG_SIZE) - 1};
    static constexpr Config DEFAULT_CONFIG
    {
        // Buttons are mapped to F5 - F8 with no modifiers
        /*.Buttons = { 0x3E, 0x00, 0x3F, 0x00, 0x40, 0x00, 0x41, 0x00, },*/
        .Buttons = { 0x68, 0x00, 0x69, 0x00, 0x6A, 0x00, 0x6B, 0x00, },
        .DebounceTime = 5,
        .SetLedMode = LedMode::NORMAL
    };
    
    static Config currentConfig_;
    static std::size_t writeIndex_;
};