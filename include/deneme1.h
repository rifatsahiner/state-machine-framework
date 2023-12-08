#ifndef DENEME1_H
#define DENEME1_H

#include "ulak.h"

// enum class Deneme1Tasks : uOS::TaskId {
//     TASK_1,
//     TASK_2,
//     TASK_LIST_COUNT
// };
namespace AppDeneme1 {


// external (inter-task) signals
enum AppSignals : uOS::SignalId {
    SIGNAL_1 = uOS::EXT_SIGNAL_START,
    SIGNAL_2,
    SIGNAL_3,
    SIGNAL_LIST_COUNT
};

//external event types
struct Event1 : uOS::Event {
    // uOS::TimerId timerId;
    // bool isCancel;
    std::string s;
};


}   // namespace AppDeneme1

#endif
