#pragma once
#include <cstdint>

enum class LedMode : std::uint8_t
{
    OFF,      // 0x00     
    ON,       // 0x01
    NORMAL,   // 0x02
    INVERTED, // 0x03
    INVALID   // <- This entry is used for range/validity checking and must always be the last entry
};