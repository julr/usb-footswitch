
#include <cstdint>
#include "USBDeecriptors.h"
#include "tusb.h"
#include "Board.h"


//--------------------------------------------------------------------+
// Device Descriptor
//--------------------------------------------------------------------+
static constexpr std::uint16_t VID {0xF055}; // obsolete USB IF VID
static constexpr std::uint16_t PID {0x59B8}; // some random number
static constexpr tusb_desc_device_t DEVICE_DESCRIPTOR
{
    .bLength = sizeof(tusb_desc_device_t),
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = 0x0200,
    .bDeviceClass = 0x00,
    .bDeviceSubClass = 0x00,
    .bDeviceProtocol = 0x00,
    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor = VID,
    .idProduct = PID,
    .bcdDevice = 0x0100,
    .iManufacturer = 0x01,
    .iProduct = 0x02,
    .iSerialNumber = 0x03,
    .bNumConfigurations = 0x01
};


//--------------------------------------------------------------------+
// HID Report Descriptor
//--------------------------------------------------------------------+
static constexpr std::uint8_t KEYBOARD_HID_DESCRIPTOR[] { TUD_HID_REPORT_DESC_KEYBOARD() };
static constexpr std::uint8_t CONTROL_HID_DESCRIPTOR[] { TUD_HID_REPORT_DESC_GENERIC_INOUT(CFG_TUD_HID_EP_BUFSIZE) };


//--------------------------------------------------------------------+
// Configuration Descriptor
//--------------------------------------------------------------------+

static constexpr unsigned INTERFACE_COUNT {2};
static constexpr std::uint8_t KEYBOARD_INTERFACE_STRING_DESCRIPTOR {4};
static constexpr std::uint8_t CONTROL_INTERFACE_STRING_DESCRIPTOR {5};

static const unsigned CONFIG_TOTAL_LEN {TUD_CONFIG_DESC_LEN + TUD_HID_DESC_LEN + TUD_HID_INOUT_DESC_LEN};

static constexpr std::uint8_t HID_KEYBOARD_ENDPOINT {0x01};
static constexpr std::uint8_t HID_CONTROL_ENDPOINT {0x02};
static constexpr std::uint8_t IN_ENDPOINT_FLAG {0x80};

static constexpr uint8_t const DESCRITOR_CONFIGURATION[] =
{
    // Config number, interface count, string index, total length, attribute, power in mA
    TUD_CONFIG_DESCRIPTOR(1, INTERFACE_COUNT, 0, CONFIG_TOTAL_LEN, TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 100),

    // Interface number, string index, protocol, report descriptor len, EP In address, size & polling interval
    TUD_HID_DESCRIPTOR(KEYBOARD_INTERFACE, KEYBOARD_INTERFACE_STRING_DESCRIPTOR, HID_ITF_PROTOCOL_NONE, sizeof(KEYBOARD_HID_DESCRIPTOR), HID_KEYBOARD_ENDPOINT | IN_ENDPOINT_FLAG, CFG_TUD_HID_EP_BUFSIZE, HID_POLL_INTERVAL),
    
    // Interface number, string index, protocol, report descriptor len, EP In & Out address, size & polling interval
    TUD_HID_INOUT_DESCRIPTOR(CONTROL_INTERFACE, CONTROL_INTERFACE_STRING_DESCRIPTOR, HID_ITF_PROTOCOL_NONE, sizeof(CONTROL_HID_DESCRIPTOR), HID_CONTROL_ENDPOINT, HID_CONTROL_ENDPOINT | IN_ENDPOINT_FLAG, CFG_TUD_HID_EP_BUFSIZE, HID_POLL_INTERVAL)
};


//--------------------------------------------------------------------+
// String Descriptors
//--------------------------------------------------------------------+
static constexpr std::uint16_t STRING_LANGUAE { 0x0409 };  // supported language is English
static constexpr char STRING_MANUFACTURER[] { "Julian" };
static constexpr char STRING_PRODUCT[] { "USB Foot Switch" };
static constexpr char STRING_KEYBOARD_INTERFACE[] { "Keyboard Interface" };
static constexpr char STRING_CONTROL_INTERFACE[] { "Control Interface" };


//--------------------------------------------------------------------+
// TinyUSB Callbacks
//--------------------------------------------------------------------+

// Invoked when received GET DEVICE DESCRIPTOR
// Application return pointer to descriptor
extern "C" std::uint8_t const * tud_descriptor_device_cb(void)
{
    return reinterpret_cast<std::uint8_t* const>( const_cast<tusb_desc_device_t* const>( &DEVICE_DESCRIPTOR ));
}

// Invoked when received GET HID REPORT DESCRIPTOR
// Application return pointer to descriptor
// Descriptor contents must exist long enough for transfer to complete
extern "C" std::uint8_t const * tud_hid_descriptor_report_cb(uint8_t itf)
{
    switch (itf)
    {
    case 0:
        return KEYBOARD_HID_DESCRIPTOR;
    case 1:
        return CONTROL_HID_DESCRIPTOR;
    default:
        return nullptr;
    }
}


// Invoked when received GET CONFIGURATION DESCRIPTOR
// Application return pointer to descriptor
// Descriptor contents must exist long enough for transfer to complete
extern "C" std::uint8_t const * tud_descriptor_configuration_cb(uint8_t index)
{
    static_cast<void>(index); // for multiple configurations, not used here
    return DESCRITOR_CONFIGURATION;
}


// Invoked when received GET STRING DESCRIPTOR request
// Application return pointer to descriptor, whose contents must exist long enough for transfer to complete
extern "C" std::uint16_t const* tud_descriptor_string_cb(uint8_t index, uint16_t langid)
{
    static_cast<void>(langid);
    
    static std::uint16_t descriptorString[32];
    std::uint8_t stringLength{1};
    const char* stringSource{nullptr};
    
    switch (index)
    {
    case 0:
        descriptorString[1] = STRING_LANGUAE;
        break;
        
    case DEVICE_DESCRIPTOR.iManufacturer:
        stringSource = STRING_MANUFACTURER;
        break;
   
    case DEVICE_DESCRIPTOR.iProduct:
        stringSource = STRING_PRODUCT;
        break;
        
    case DEVICE_DESCRIPTOR.iSerialNumber:
        stringSource = Board::getSerialNumber();
        break;
        
    case KEYBOARD_INTERFACE_STRING_DESCRIPTOR:
        stringSource = STRING_KEYBOARD_INTERFACE;
        break;
        
    case CONTROL_INTERFACE_STRING_DESCRIPTOR:
        stringSource = STRING_CONTROL_INTERFACE;
        break;
        
    default:
        return nullptr;
    }

    if(stringSource != nullptr)
    {
        // Cap at max char
        stringLength = strlen(stringSource);
        if (stringLength > 31) stringLength = 31;

        // Convert ASCII string into UTF-16
        for (uint8_t i = 0; i < stringLength; i++)
        {
            descriptorString[1 + i] = stringSource[i];
        }
    }

    // first byte is length (including header), second byte is string type
    descriptorString[0] = (TUSB_DESC_STRING << 8) | (2*stringLength + 2);

    return descriptorString;
}
