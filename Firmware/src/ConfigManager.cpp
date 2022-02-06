#include "ConfigManager.h"
#include <cstring>

Config ConfigManager::currentConfig_;

constexpr Config ConfigManager::DEFAULT_CONFIG;
std::size_t ConfigManager::writeIndex_{0};

void ConfigManager::initialize()
{
    const Config* configFlashPtr{reinterpret_cast<Config*>(Board::USER_FLASH_SECTION_START)};
    // first check if the first config is valid, if not load the default
    if ((*configFlashPtr).SetLedMode >= LedMode::INVALID)
    {
        std::memcpy(&currentConfig_, &DEFAULT_CONFIG, CONFIG_SIZE);
    }
    else
    {
        std::uint32_t currentConfigIndex{0};
        for (; currentConfigIndex <= LAST_CONFIG_INDEX; currentConfigIndex++) // search for the first valid config
        {
            if ((configFlashPtr[currentConfigIndex]).SetLedMode >= LedMode::INVALID)
                break;
        }
        // Load the config
        std::memcpy(&currentConfig_, &configFlashPtr[currentConfigIndex-1], CONFIG_SIZE);
        writeIndex_ = currentConfigIndex; 
    }    
}

bool ConfigManager::setButtonConfig(std::size_t button, std::uint8_t scanCode, std::uint8_t modifier)
{
    if (button > 3) return false;
    currentConfig_.Buttons[button].ScanCode = scanCode;
    currentConfig_.Buttons[button].Modifier = modifier;
    return true;
}
    
void ConfigManager::setButtonDebounceTime(std::uint8_t debounceTime)
{
    currentConfig_.DebounceTime = debounceTime;
}

bool ConfigManager::setLedMode(LedMode ledMode)
{
    if (ledMode >= LedMode::INVALID) return false;
    currentConfig_.SetLedMode = ledMode;
    return true;
}

bool ConfigManager::saveConfig()
{
    //first check if we even need to save
    if (writeIndex_ > 0)
    {
        const Config* currentConfigInFlashPtr{reinterpret_cast<Config*>(Board::USER_FLASH_SECTION_START) + writeIndex_ - 1};
        if (std::memcmp(&currentConfig_, currentConfigInFlashPtr, CONFIG_SIZE) == 0) return true;
    }
    
    // do we need to erase the flash first?
    if (writeIndex_ > LAST_CONFIG_INDEX)
    {
        if (!Board::eraseUserFlash()) return false;
        writeIndex_ = 0;
    }

    return Board::writeUserFlash(Board::USER_FLASH_SECTION_START + (writeIndex_ * CONFIG_SIZE), &currentConfig_, CONFIG_SIZE);
}
