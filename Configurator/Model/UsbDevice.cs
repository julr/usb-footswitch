using Helpers;
using HidSharp;
using System;
using System.Collections.Generic;
using System.Diagnostics.CodeAnalysis;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Configurator.Model
{
    internal class UsbDevice
    {
        public enum LedMode
        {
            OFF = 0x00,     
            ON = 0x01,
            NORMAL = 0x02,
            INVERTED = 0x03
        }

        internal readonly struct DeviceInfo
        {
            public int FirmwareVersionMajor { get; init; }
            public int FirmwareVersionMinor { get; init; }
            public int ButtonCount { get; init; }
        }

        internal class ButtonConfiguration : ObservableObject
        {
            private ScanCode key_;
            public ScanCode Key { get => key_; set => SetProperty(ref key_, value); }
            private bool modifierLeftControl_;
            public bool ModifierLeftControl { get => modifierLeftControl_; set => SetProperty(ref modifierLeftControl_, value); }
            private bool modifierLeftShift_;
            public bool ModifierLeftShift { get => modifierLeftShift_; set => SetProperty(ref modifierLeftShift_, value); }
            private bool modifierLeftAlt_;
            public bool ModifierLeftAlt { get => modifierLeftAlt_; set => SetProperty(ref modifierLeftAlt_, value); }
            private bool modifierLeftGui_;
            public bool ModifierLeftGui { get => modifierLeftGui_; set => SetProperty(ref modifierLeftGui_, value); }
            private bool modifierRightControl_;
            public bool ModifierRightControl { get => modifierRightControl_; set => SetProperty(ref modifierRightControl_, value); }
            private bool modifierRightShift_;
            public bool ModifierRightShift { get => modifierRightShift_; set => SetProperty(ref modifierRightShift_, value); }
            private bool modifierRightAlt_;
            public bool ModifierRightAlt { get => modifierRightAlt_; set => SetProperty(ref modifierRightAlt_, value); }
            private bool modifierRightGui_;
            public bool ModifierRightGui { get => modifierRightGui_; set => SetProperty(ref modifierRightGui_, value); }
        }

        public string SerialNumber { get; private set; }

        [AllowNullAttribute]
        private HidDevice hidDevice;

        [AllowNullAttribute]
        private HidStream hidStream;

        private enum ControlCommand : byte
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
        }

        private enum ControlReponse : byte
        {
            DEVICE_INFO_RESPONSE = 0x80,
            GET_BUTTON_CONFIGURATION_RESPONSE = 0x81,
            SET_BUTTON_CONFIGURATION_RESPONSE = 0x82,
            GET_BUTTON_DEBOUNCE_RESPONSE = 0x83,
            SET_BUTTON_DEBOUNCE_RESPONSE = 0x84,
            GET_LED_CONFIGURATION_RESPONSE = 0x85,
            SET_LED_CONFIGURATION_RESPONSE = 0x86,
            SAVE_CONFIGURATION_RESPONSE = 0x87
        }

        public UsbDevice()
        {
            SerialNumber = "";
        }



        public void Open(int vid = 0xF055, int pid = 0x59B8, string serial = "")
        {
            var list = DeviceList.Local;
            //The Library does not allow to select a specific interface or filter by usage
            //Workaround -> The hid control interface has the same EP size for in and output, the hid keyboard interface has different sizes
            var devices = list.GetHidDevices(vid, pid).Where(d => d.GetMaxInputReportLength() == d.GetMaxOutputReportLength());
            if(devices != null)
            {
                hidDevice = (serial.Length > 0) ? devices.FirstOrDefault(d => d.GetSerialNumber() == serial) : devices.FirstOrDefault();

                if ((hidDevice != null) && hidDevice.TryOpen(out hidStream))
                {
                    SerialNumber = hidDevice.GetSerialNumber();
                }
                else
                {
                    throw new Exception("No device found");
                }
            }
        }

        public void StartBootloader()
        {
            SendCommand(ControlCommand.START_BOOTLOADER_REQUEST, Array.Empty<byte>(), out _, false);
        }

        public DeviceInfo GetDeviceInfo()
        {
            SendCommandAndCheck(ControlCommand.DEVICE_INFO_REQUEST, ControlReponse.DEVICE_INFO_RESPONSE, 5, Array.Empty<byte>(), out byte[] returnData);

            DeviceInfo deviceInfo = new()
            {
                FirmwareVersionMajor = returnData[2],
                FirmwareVersionMinor = returnData[3],
                ButtonCount = returnData[4]
            };
            return deviceInfo;
        }

        public LedMode GetLedMode()
        {
            SendCommandAndCheck(ControlCommand.GET_LED_CONFIGURATION_REQUEST, ControlReponse.GET_LED_CONFIGURATION_RESPONSE, 3, Array.Empty<byte>(), out byte[] returnData);

            return (LedMode)returnData[2];
        }

        public int GetDebounceTime()
        {
            SendCommandAndCheck(ControlCommand.GET_BUTTON_DEBOUNCE_REQUEST, ControlReponse.GET_BUTTON_DEBOUNCE_RESPONSE, 3, Array.Empty<byte>(), out byte[] returnData);

            return returnData[2];
        }

        public ButtonConfiguration GetButtonConfigration(int button)
        {
            SendCommandAndCheck(ControlCommand.GET_BUTTON_CONFIGURATION_REQUEST, ControlReponse.GET_BUTTON_CONFIGURATION_RESPONSE, 3, new byte[] { (byte)button }, out byte[] returnData);

            ButtonConfiguration buttonConfigration = new()
            {
                Key = (ScanCode)returnData[3],
                ModifierLeftControl = (returnData[4] & 0x01) != 0,
                ModifierLeftShift = (returnData[4] & 0x02) != 0,
                ModifierLeftAlt = (returnData[4] & 0x04) != 0,
                ModifierLeftGui = (returnData[4] & 0x08) != 0,
                ModifierRightControl = (returnData[4] & 0x10) != 0,
                ModifierRightShift = (returnData[4] & 0x20) != 0,
                ModifierRightAlt = (returnData[4] & 0x40) != 0,
                ModifierRightGui = (returnData[4] & 0x80) != 0
            };

            return buttonConfigration;
        }


        public void SetLedMode(LedMode ledMode)
        {
            SendCommandAndCheck(ControlCommand.SET_LED_CONFIGURATION_REQUEST, ControlReponse.SET_LED_CONFIGURATION_RESPONSE, 2, new byte[] { (byte)ledMode }, out _);
        }

        public void SetDebounceTime(byte debounceTime)
        {
            SendCommandAndCheck(ControlCommand.SET_BUTTON_DEBOUNCE_REQUEST, ControlReponse.SET_BUTTON_DEBOUNCE_RESPONSE, 2, new byte[] { (byte)debounceTime }, out _);
        }

        public void SetButtonConfiguration(int button, ButtonConfiguration configuration)
        {
            byte[] payload = new byte[] { (byte)button, (byte)configuration.Key, 0 };
            if (configuration.ModifierLeftControl) payload[2] |= 0x01;
            if (configuration.ModifierLeftShift) payload[2] |= 0x02;
            if (configuration.ModifierLeftAlt) payload[2] |= 0x04;
            if (configuration.ModifierLeftGui) payload[2] |= 0x08;
            if (configuration.ModifierRightControl) payload[2] |= 0x10;
            if (configuration.ModifierRightShift) payload[2] |= 0x20;
            if (configuration.ModifierRightAlt) payload[2] |= 0x40;
            if (configuration.ModifierRightGui) payload[2] |= 0x80;
            SendCommandAndCheck(ControlCommand.SET_BUTTON_CONFIGURATION_REQUEST, ControlReponse.SET_BUTTON_CONFIGURATION_RESPONSE, 2, payload, out _);
        }

        public void SaveConfiguration()
        {
            SendCommandAndCheck(ControlCommand.SAVE_CONFIGURATION_REQUEST, ControlReponse.SAVE_CONFIGURATION_RESPONSE, 2, Array.Empty<byte>(), out _);
        }

        private void SendCommandAndCheck(ControlCommand command, ControlReponse expectedResponse, int expectedMinialLength, byte[] payload, out byte[] returnPayload)
        {
            SendCommand(command, payload, out returnPayload);
            if (returnPayload.Length < expectedMinialLength)
                throw new Exception("Received device info is too short");
            if (returnPayload[0] != (byte)expectedResponse)
                throw new Exception("Device returned error code: " + returnPayload[0]);
            if (returnPayload[1] != 0)
                throw new Exception("Device returned error code: " + returnPayload[1]);
        }

        private void SendCommand(ControlCommand command, byte[] payload, out byte[] returnPayload, bool readResult = true)
        {
            if ((hidDevice == null) || (hidDevice == null))
                throw new InvalidOperationException("Device note opened");

            if (payload.Length > (hidDevice.GetMaxOutputReportLength() - 2))
                throw new ArgumentException("Payload is too big");

            var inputReportBuffer = new byte[hidDevice.GetMaxInputReportLength()];
            var outputReportBuffer = new byte[hidDevice.GetMaxOutputReportLength()];

            outputReportBuffer[0] = 0x00; //report id is byte 0 is always 0 (only one interface)
            outputReportBuffer[1] = (byte)command;
            if(payload.Length > 0)
                payload.CopyTo(outputReportBuffer, 2);
            hidStream.Write(outputReportBuffer);

            if(readResult)
            {
                hidStream.Read(inputReportBuffer);
                returnPayload = inputReportBuffer.Skip(1).ToArray(); //skip interface byte (always 0)
            }
            else
            {
                returnPayload = Array.Empty<byte>();
            }
        }
    }
}
