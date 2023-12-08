
#include "deneme1.h"
#include "task1.h"
#include <iostream>

//extern uOS::TaskId g_task2Id;

// internal signals
enum Task1signals : uOS::SignalId {
    PUBLISH,
    SIGNAL_LIST_COUNT
};

Task1::Task1(const uOS::TaskId taskId, const std::string& taskName) : Task (taskId, taskName) { }

void Task1::_stateMachine(std::shared_ptr<const uOS::Event>& receivedEventBase) 
{
    std::cout << "_stateMachine before_cast - event copy count = " << receivedEventBase.use_count() << std::endl;

    // switch(receivedEventBase->signal) 
    // {
    //     case Task1signals::PUBLISH:
    //     {
    //         auto event = uOS::new_e<AppDeneme1::Event1>(AppDeneme1::AppSignals::SIGNAL_1);
    //         event->s = "this is event from task 1\n";
    //         uOS::FW::publishEvent(event);
    //     }
    //     break;
        
    //     case AppDeneme1::AppSignals::SIGNAL_2:
    //     case AppDeneme1::AppSignals::SIGNAL_3:
    //         auto event1 = uOS::recast_e<AppDeneme1::Event1>(receivedEventBase);
    //         std::cout << event1->s << std::endl;
    //     break;

    // }

    if(receivedEventBase->signal == AppDeneme1::AppSignals::SIGNAL_1){
        std::cout << "this is task1 - event: " << ++_count << std::endl;
        
        auto event1 = uOS::recast_e<AppDeneme1::Event1>(receivedEventBase);
        std::cout << "_stateMachine after_cast - event copy count = " << receivedEventBase.use_count() << std::endl;
        std::cout << event1->s << std::endl;

        if(_count == 5){
            unsubscribe(AppDeneme1::AppSignals::SIGNAL_1);
            std::cout << "task1 - unsubscribed " << std::endl;
        }

        // auto event1 = uOS::recast_e<AppDeneme1::Event1>(receivedEventBase);
        // std::cout << "_stateMachine after_cast - event copy count = " << receivedEventBase.use_count() << std::endl;
        // std::cout << event1->s << std::endl;
    }
}

void Task1::_init(void)
{
    // subscribe(AppDeneme1::AppSignals::SIGNAL_2);
    // subscribe(AppDeneme1::AppSignals::SIGNAL_3);

    // auto event = uOS::new_e<uOS::Event>(Task1signals::PUBLISH);
    // uOS::FW::postEventIn(11000, _taskId, event);

    subscribe(AppDeneme1::AppSignals::SIGNAL_1);
}
