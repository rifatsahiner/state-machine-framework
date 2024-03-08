
#include <iostream>
#include "uOS.h"
#include "App1.h"
#include "key_reader.h"



namespace App1 {


static volatile bool _isRunning = true;
// extern uOS::TaskId g_task1Id;

void keyReadThread(void)
{
    while(_isRunning){
        char ch;
        uOS::SignalId signal;

        std::cout << "press key for firing event\n" << std::endl;
        std::cin >> ch;

        if((ch >= 'a') && (ch <= 'i')){
            signal = static_cast<uOS::SignalId>(ch - 'a') + SIGNAL_A;
        } else if((ch >= 'A') && (ch <= 'I')) {
            signal = static_cast<uOS::SignalId>(ch - 'A') + SIGNAL_A;
        } else if (ch == 27){
            signal = TERMINATE;
        } else {
            signal = IGNORE;
        }

        auto event = uOS::new_e(signal);
        uOS::FW::postEvent(g_hsmTestId, event);
    }

    std::cout << "key reader stopped" << std::endl;
}

void keyReaderStop(void){
    _isRunning = false;
}


}
