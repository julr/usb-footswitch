using Configurator.Model;
using Helpers;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Input;

namespace Configurator.ViewModels
{
    internal class MainWindowViewModel : ObservableObject
    {
        private static readonly int MAX_BUTTONS = 4;

        public string FirmwareVersion { get; private set; }
        public string SerialNumber { get; private set; }
        public int NumberOfButtons { get; private set; }

        public bool[] EnabledButtons { get; private set; }

        public UsbDevice.ButtonConfiguration[] ButtonConfigurations { get; private set; }

        private UsbDevice.LedMode ledMode_;
        public UsbDevice.LedMode LedMode { get => ledMode_; set => SetProperty(ref ledMode_, value); }

        private int debounceTime_;
        public int DebounceTime { get => debounceTime_; set => SetProperty(ref debounceTime_, value); }

        public IEnumerable<UsbDevice.LedMode> PossibleLedModes { get; private set; } = Enum.GetValues(typeof(UsbDevice.LedMode)).Cast<UsbDevice.LedMode>().ToArray();
        public IEnumerable<ScanCode> PossibleKeys { get; private set; } = Enum.GetValues(typeof(ScanCode)).Cast<ScanCode>().ToArray();

        private readonly UsbDevice usbDevice;
        public MainWindowViewModel()
        {
            usbDevice = new UsbDevice();
            try
            {
                usbDevice.Open();
                var deviceInfo = usbDevice.GetDeviceInfo();
                FirmwareVersion = $"{deviceInfo.FirmwareVersionMajor}.{deviceInfo.FirmwareVersionMinor:D2}";
                SerialNumber = usbDevice.SerialNumber;
                NumberOfButtons = deviceInfo.ButtonCount;
                LedMode = usbDevice.GetLedMode();
                DebounceTime = usbDevice.GetDebounceTime();
                EnabledButtons = new bool[MAX_BUTTONS];
                ButtonConfigurations = new UsbDevice.ButtonConfiguration[MAX_BUTTONS];
                for (int i = 0; i < NumberOfButtons; i++)
                {
                    EnabledButtons[i] = true;
                    ButtonConfigurations[i] = usbDevice.GetButtonConfigration(i);
                }
            }
            catch(Exception ex)
            {
                System.Windows.MessageBox.Show(ex.Message, "Error", System.Windows.MessageBoxButton.OK, System.Windows.MessageBoxImage.Error);
                App.Current.Shutdown();
            }
        }

        public ICommand StartBootloader => new Command(_ =>
        {
            usbDevice.StartBootloader();
            System.Windows.MessageBox.Show("Bootloader started, this application will now close", "Success");
            App.Current.Shutdown();
        });

        public ICommand ApplySettings => new Command(_ =>
        {
            if(DebounceTime > 255) DebounceTime = 255;
            if(DebounceTime < 0) DebounceTime = 0;
            try
            {
                usbDevice.SetLedMode(LedMode);
                usbDevice.SetDebounceTime((byte)DebounceTime);
                for(int i = 0; i<NumberOfButtons; i++)
                    usbDevice.SetButtonConfiguration(i, ButtonConfigurations[i]);
                System.Windows.MessageBox.Show("Settings applied.\nSave them if you want to store them permanently on the device.", "Success", System.Windows.MessageBoxButton.OK, System.Windows.MessageBoxImage.Information);
            }
            catch (Exception ex)
            {
                System.Windows.MessageBox.Show(ex.Message, "Error", System.Windows.MessageBoxButton.OK, System.Windows.MessageBoxImage.Error);
            }
        });

        public ICommand SaveSettings => new Command(_ =>
        {
            try
            {
                usbDevice.SaveConfiguration();
                System.Windows.MessageBox.Show("Settings saved.", "Success", System.Windows.MessageBoxButton.OK, System.Windows.MessageBoxImage.Information);
            }
            catch (Exception ex)
            {
                System.Windows.MessageBox.Show(ex.Message, "Error", System.Windows.MessageBoxButton.OK, System.Windows.MessageBoxImage.Error);
            }
        });

    }
}
