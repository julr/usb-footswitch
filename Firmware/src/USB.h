#pragma once
#include <cstdint>
#include "tusb.h"
#include "USBDeecriptors.h"

class USB
{
public:
    friend void(::tud_mount_cb)();
    friend void(::tud_umount_cb)();
    friend void(::tud_suspend_cb)(bool);
    friend void(::tud_resume_cb)();
    friend void(::tud_hid_set_report_cb)(uint8_t, uint8_t, hid_report_type_t, uint8_t const*, uint16_t);
    
    static void initialize(bool useStatusLed = true);
    static void task();
    
private:
    enum class LedBlinkTiming : std::uint32_t //Timing in ms
    {
        NOT_MOUNTED = 250,  // device not mounted
        MOUNTED     = 1000, // device mounted
        SUSPENDED   = 2500  // device suspended
    };
    
    enum class ControlCommand : std::uint8_t
    {
        
        DEVICE_INFO_REQUEST = 0x00,
        GET_BUTTON_CONFIGURATION_REQUEST = 0x01,
        SET_BUTTON_CONFIGURATION_REQUEST = 0x02,
        GET_BUTTON_DEBOUNCE_REQUEST = 0x03,
        SET_BUTTON_DEBOUNCE_REQUEST = 0x04,
        GET_LED_CONFIGURATION_REQUEST = 0x05,
        SET_LED_CONFIGURATION_REQUEST = 0x06,
        SAVE_CONFIGURATION_REQUEST = 0x07,
        START_BOOTLOADER_REQUEST = 0x7F        
    };
    
    enum class ControlResponse : std::uint8_t
    {
        DEVICE_INFO_RESPONSE = 0x80,
        GET_BUTTON_CONFIGURATION_RESPONSE = 0x81,
        SET_BUTTON_CONFIGURATION_RESPONSE = 0x82,
        GET_BUTTON_DEBOUNCE_RESPONSE = 0x83,
        SET_BUTTON_DEBOUNCE_RESPONSE = 0x84,
        GET_LED_CONFIGURATION_RESPONSE = 0x85,
        SET_LED_CONFIGURATION_RESPONSE = 0x86,
        SAVE_CONFIGURATION_RESPONSE = 0x87
    };
    
    
enum class ControlReturnCode : std::uint8_t
    {
        OK = 0x00,
        UNKOWN_COMMAND = 0x01,
        BUTTON_OUT_OF_RANGE = 0x02,
        UNKOWN_LED_MODE = 0x03,
        SAVE_OPERATION_FAILED = 0x04
    };
    
    static void hidTask();
    static void ledTask();
    
    static bool useLed_;
    static std::uint32_t ledBlinkInterval_;
    static bool wasButtonPressed_;
    
    static bool sendControlMessage_;
    
    static std::uint8_t controlMessage_[CFG_TUD_HID_EP_BUFSIZE];
};