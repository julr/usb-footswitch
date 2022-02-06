#pragma once
#include <cstdint>
#include "LedMode.h"

struct Config
{
    struct
    {
        std::uint8_t ScanCode;
        std::uint8_t Modifier;
    }Buttons[4];
    std::uint8_t DebounceTime;
    LedMode SetLedMode;    
} __attribute__((packed));
