
#include "App1.h"
#include "task1.h"
#include <iostream>


namespace App1 {


// internal signals
// enum Task1signals : uOS::SignalId {
//     PUBLISH,
//     SIGNAL_LIST_COUNT
// };

Task1::Task1(const uOS::TaskId taskId, const std::string& taskName, const int& i, const float& f, const std::string& s) : Task (taskId, taskName), _i{i}, _f{f}, _s{s} {}

uOS::HandleResult Task1::_topInitialTrans(void)
{
    // subscribe(App1::AppSignals::SIGNAL_2);
    // subscribe(App1::AppSignals::SIGNAL_3);

    // auto event = uOS::new_e<uOS::Event>(Task1signals::PUBLISH);
    // uOS::FW::postEventIn(11000, _taskId, event);

    //subscribe(App1::AppSignals::SIGNAL_1);
    
    std::cout << "int: " << _i << "float: " << _f << "string " << _s << std::endl;

    //uOS::StateMachine::State deneme = static_cast<uOS::StateMachine::State>(&Task1::state1);

    //return retTrans(static_cast<uOS::StateMachine::State>(&Task1::state1));
    //return retSuper(static_cast<uOS::StateMachine::State>(&Task1::state1));
    //return retTrans(&Task1::state1);
    return trans(&Task1::state1);
}

uOS::HandleResult Task1::state1(const std::shared_ptr<const uOS::Event>& event) {
    std::cout << "Task1 - STATE1 function entry. signal: " << (unsigned)event->signal << std::endl;

    uOS::HandleResult status;
    switch (event->signal) {
        case AppSignals::TRANS_TO_2: {
            _i = 222;
            std::cout << "i: " << _i << std::endl;
            std::cout << "Task1 - STATE1 function - transition to -> STATE-2" << std::endl;
            status = trans(&Task1::state2);
            break;
        }

        default: {
            status = super(&Task1::top);
            break;
        }
    }
    return status;
}
        
uOS::HandleResult Task1::state2(const std::shared_ptr<const uOS::Event>& event){
    std::cout << "Task1 - STATE2 function entry. signal: " << (unsigned)event->signal << std::endl;
    
    uOS::HandleResult status;
    switch (event->signal) {
        case AppSignals::TRANS_TO_1: {
            _i = 111;
            std::cout << "i: " << _i << std::endl;
            std::cout << "Task1 - STATE2 function - transition to -> STATE-1" << std::endl;
            status = trans(&Task1::state1);
            break;
        }

        default: {
            status = super(&Task1::top);
            break;
        }
    }
    return status;
}


}   // namespace App1
