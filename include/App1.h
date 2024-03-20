#ifndef APP1_H
#define APP1_H

#include "uOS.h"

//extern uOS::TaskId g_task1Id, g_task2Id, g_task3Id;

namespace App1 {

extern uOS::TaskId g_simpleTaskId;

// external (inter-task) signals
// enum AppSignals : uOS::SignalId
// {
//     SIGNAL_A = uOS::EXT_SIGNAL_START,
//     SIGNAL_B,
//     SIGNAL_C,
//     SIGNAL_D,
//     SIGNAL_E,
//     SIGNAL_F,
//     SIGNAL_G,
//     SIGNAL_H,
//     SIGNAL_I,
//     TERMINATE,
//     IGNORE,
//     SIGNAL_LIST_COUNT
// };

//external event types
// struct Event1 : uOS::Event {
//     // uOS::TimerId timerId;
//     // bool isCancel;
//     std::string s;
// };


}   // namespace App1

#endif
