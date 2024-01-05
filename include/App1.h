#ifndef DENEME1_H
#define DENEME1_H

#include "uOS.h"

//extern uOS::TaskId g_task1Id, g_task2Id, g_task3Id;


namespace App1 {


// external (inter-task) signals
enum AppSignals : uOS::SignalId {
    TRANS_TO_1 = uOS::EXT_SIGNAL_START,
    TRANS_TO_2,
    SIGNAL_LIST_COUNT
};

//external event types
struct Event1 : uOS::Event {
    // uOS::TimerId timerId;
    // bool isCancel;
    std::string s;
};


}   // namespace App1


#endif
