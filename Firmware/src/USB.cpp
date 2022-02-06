#include "USB.h"

#include "Board.h"
#include "ConfigManager.h"
#include "Version.h"

bool USB::useLed_{false};
std::uint32_t USB::ledBlinkInterval_{static_cast<std::uint32_t>(USB::LedBlinkTiming::NOT_MOUNTED)};
bool USB::wasButtonPressed_{false};

bool USB::sendControlMessage_{false};
    
std::uint8_t USB::controlMessage_[CFG_TUD_HID_EP_BUFSIZE];

//--------------------------------------------------------------------+
// TinyUSB callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted
extern "C" void tud_mount_cb()
{
    USB::ledBlinkInterval_ = static_cast<std::uint32_t>(USB::LedBlinkTiming::MOUNTED);
}

// Invoked when device is unmounted
extern "C" void tud_umount_cb()
{
    USB::ledBlinkInterval_ = static_cast<std::uint32_t>(USB::LedBlinkTiming::NOT_MOUNTED);
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
extern "C" void tud_suspend_cb(bool remote_wakeup_en)
{
    static_cast<void>(remote_wakeup_en);
    USB::ledBlinkInterval_ = static_cast<std::uint32_t>(USB::LedBlinkTiming::SUSPENDED);
}

// Invoked when usb bus is resumed
extern "C" void tud_resume_cb()
{
    USB::ledBlinkInterval_ = static_cast<std::uint32_t>(USB::LedBlinkTiming::MOUNTED);
}

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
extern "C" uint16_t tud_hid_get_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen)
{
    // TODO not Implemented
    static_cast<void>(itf);
    static_cast<void>(report_id);
    static_cast<void>(report_type);
    static_cast<void>(buffer);
    static_cast<void>(reqlen);

    return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
extern "C" void tud_hid_set_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize)
{
    static_cast<void>(report_id);
    static_cast<void>(report_type);
    static_cast<void>(bufsize);

    if (itf == CONTROL_INTERFACE)
    {
        USB::controlMessage_[1] = static_cast<std::uint8_t>(USB::ControlReturnCode::OK);
        const Config& config{ConfigManager::getConfig()};
        
        switch (static_cast<USB::ControlCommand>(buffer[0]))
        {
        case USB::ControlCommand::DEVICE_INFO_REQUEST:
            USB::controlMessage_[0] = static_cast<std::uint8_t>(USB::ControlResponse::DEVICE_INFO_RESPONSE);
            USB::controlMessage_[2] = VersionMajor;
            USB::controlMessage_[3] = VersionMinor;
            USB::controlMessage_[4] = static_cast<std::uint8_t>(Board::getNumberOfButtons());            
            break;
            
        case USB::ControlCommand::GET_BUTTON_CONFIGURATION_REQUEST:
            USB::controlMessage_[0] = static_cast<std::uint8_t>(USB::ControlResponse::GET_BUTTON_CONFIGURATION_RESPONSE);
            if (buffer[1] < Board::getNumberOfButtons())
            {
                USB::controlMessage_[2] = buffer[1];
                USB::controlMessage_[3] = config.Buttons[buffer[1]].ScanCode;
                USB::controlMessage_[4] = config.Buttons[buffer[1]].Modifier;
            }
            else
            {
                USB::controlMessage_[1] = static_cast<std::uint8_t>(USB::ControlReturnCode::BUTTON_OUT_OF_RANGE);
            }
            break;
            
        case USB::ControlCommand::SET_BUTTON_CONFIGURATION_REQUEST:
            USB::controlMessage_[0] = static_cast<std::uint8_t>(USB::ControlResponse::SET_BUTTON_CONFIGURATION_RESPONSE);
            USB::controlMessage_[2] = buffer[1];
            if ((buffer[1] >= Board::getNumberOfButtons()) || (!ConfigManager::setButtonConfig(buffer[1], buffer[2], buffer[3])))
            {
                USB::controlMessage_[1] = static_cast<std::uint8_t>(USB::ControlReturnCode::BUTTON_OUT_OF_RANGE);
            }
            break;
            
        case USB::ControlCommand::GET_BUTTON_DEBOUNCE_REQUEST:
            USB::controlMessage_[0] = static_cast<std::uint8_t>(USB::ControlResponse::GET_BUTTON_DEBOUNCE_RESPONSE);
            USB::controlMessage_[2] = static_cast<std::uint8_t>(config.DebounceTime);
            break;
            
        case USB::ControlCommand::SET_BUTTON_DEBOUNCE_REQUEST:
            USB::controlMessage_[0] = static_cast<std::uint8_t>(USB::ControlResponse::SET_BUTTON_DEBOUNCE_RESPONSE);
            ConfigManager::setButtonDebounceTime(buffer[1]);
            Board::resetButtonStates();
            break;
            
        case USB::ControlCommand::GET_LED_CONFIGURATION_REQUEST:
            USB::controlMessage_[0] = static_cast<std::uint8_t>(USB::ControlResponse::GET_LED_CONFIGURATION_RESPONSE);
            USB::controlMessage_[2] = static_cast<std::uint8_t>(config.SetLedMode);
            break;
            
        case USB::ControlCommand::SET_LED_CONFIGURATION_REQUEST:
            USB::controlMessage_[0] = static_cast<std::uint8_t>(USB::ControlResponse::SET_LED_CONFIGURATION_RESPONSE);
            if (!ConfigManager::setLedMode(static_cast<LedMode>(buffer[1])))
            {
                USB::controlMessage_[1] = static_cast<std::uint8_t>(USB::ControlReturnCode::UNKOWN_LED_MODE);
            }
            else
            {
                Board::resetButtonStates();
            }
            break;
            
        case USB::ControlCommand::SAVE_CONFIGURATION_REQUEST:
            USB::controlMessage_[0] = static_cast<std::uint8_t>(USB::ControlResponse::SAVE_CONFIGURATION_RESPONSE);
            if (!ConfigManager::saveConfig())
            {
                USB::controlMessage_[1] = static_cast<std::uint8_t>(USB::ControlReturnCode::SAVE_OPERATION_FAILED);
            }
            break;
            
        case USB::ControlCommand::START_BOOTLOADER_REQUEST:
        {
            tud_disconnect();
            const std::uint32_t startTime{Board::getMillis()}; 
            while (Board::getMillis() - startTime < 2500) __asm("nop"); // wait 2.5 s for USB disconnect
            Board::startBootloader();
            break;
        }
                
        default:
            USB::controlMessage_[0] = 0xFF;
            USB::controlMessage_[1] = static_cast<std::uint8_t>(USB::ControlReturnCode::UNKOWN_COMMAND);
            break;
        }

        USB::sendControlMessage_ = true;
    }
}

void USB::initialize(bool useStatusLed /* = true */)
{
    tusb_init();
    useLed_ = useStatusLed;
}


void USB::task()
{
    tud_task(); // tinyusb device task
    hidTask();
    if (useLed_)
    {
        ledTask();
    }
}


void USB::hidTask()
{
    static std::uint32_t requestTime{0};

    if (Board::getMillis() - requestTime < HID_POLL_INTERVAL) return; // not enough time passed between possible requests
    requestTime += HID_POLL_INTERVAL;
    
    const Config& config{ConfigManager::getConfig()};
    const std::size_t maxButtons{Board::getNumberOfButtons()};
    
    bool anyButtonPressed{false};
    for (std::size_t i = 0; i < maxButtons; i++)
    {
        anyButtonPressed = anyButtonPressed || Board::getButtonState(static_cast<Button>(static_cast<std::size_t>(Button::FOOT_1) + i));
    }

    // Remote wakeup
    if (tud_suspended() && anyButtonPressed)
    {
        // Wake up host if we are in suspend mode
        // and REMOTE_WAKEUP feature is enabled by host
        tud_remote_wakeup();
    }

    /*------------- Keyboard -------------*/
    if (tud_hid_n_ready(KEYBOARD_INTERFACE))
    {
        // use the wasButtonPressed_ flag to avoid sending multiple consecutive zero reports
        if (anyButtonPressed)
        {
            std::uint8_t keycodes[6]{0};
            std::uint8_t modifiers{0};
            std::size_t currentKeyCode{0};
            for (std::size_t i = 0; i < maxButtons; i++) //Note: for more than 6 buttons this has to be changed
            {
                if (Board::getButtonState(static_cast<Button>(static_cast<std::size_t>(Button::FOOT_1) + i)))
                {
                    modifiers |= config.Buttons[i].Modifier;
                    const std::uint8_t scanCode{config.Buttons[i].ScanCode};
                    if (scanCode != 0x00) // 0 = KEY_NONE 
                    {
                        keycodes[currentKeyCode++] = scanCode;
                    }
                }
            }
            tud_hid_n_keyboard_report(KEYBOARD_INTERFACE, 0, modifiers, keycodes);
            wasButtonPressed_ = true;
        }
        // send empty key report if the button was just released
        else if (wasButtonPressed_)
        {
            tud_hid_n_keyboard_report(KEYBOARD_INTERFACE, 0, 0, nullptr);
            wasButtonPressed_ = false;
        }
    }

    /*------------- Control -------------*/
    if (tud_hid_n_ready(CONTROL_INTERFACE))
    {
        if (sendControlMessage_)
        {
            tud_hid_n_report(CONTROL_INTERFACE, 0, controlMessage_, sizeof(controlMessage_));
            sendControlMessage_ = false;
        }
    }
}

void USB::ledTask()
{
    static std::uint32_t ledTime{0};
    static bool newLedState{false};

    // Blink every interval ms
    if (Board::getMillis() - ledTime < ledBlinkInterval_) return; // not enough time has passed
    ledTime += ledBlinkInterval_;

    Board::setLed(LED::ONBOARD, newLedState);
    newLedState = !newLedState;
}