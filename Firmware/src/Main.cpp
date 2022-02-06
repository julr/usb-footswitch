#include "Board.h"
#include "ConfigManager.h"
#include "USB.h"

int main(void)
{   
    Board::initialize();
    ConfigManager::initialize();
#ifdef DEBUG
    USB::initialize();
#else
    USB::initialize(false); // Disable onboard LED for release builds
#endif
    Board::resetButtonStates();
    
    while (true)
    {
        USB::task();
    }

    return 0;
}