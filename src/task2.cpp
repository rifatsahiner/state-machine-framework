
#include "deneme1.h"
#include "task2.h"
#include <iostream>

//extern uOS::TaskId g_task2Id;

// internal signals
enum Task2signals : uOS::SignalId {
    PUBLISH,
    SIGNAL_LIST_COUNT
};

Task2::Task2(const uOS::TaskId taskId, const std::string& taskName) : Task(taskId, taskName) { }

void Task2::_stateMachine(std::shared_ptr<const uOS::Event>& receivedEventBase) 
{
    // switch(receivedEventBase->signal) 
    // {
    //     case Task2signals::PUBLISH: {
    //         auto event = uOS::new_e<AppDeneme1::Event1>(AppDeneme1::AppSignals::SIGNAL_2);
    //         event->s = "this is event from task 2\n";
    //         uOS::FW::publishEvent(event); }
    //     break;
        
    //     case AppDeneme1::AppSignals::SIGNAL_1:
    //     case AppDeneme1::AppSignals::SIGNAL_3:
    //         auto event1 = uOS::recast_e<AppDeneme1::Event1>(receivedEventBase);
    //         std::cout << event1->s << std::endl;
    //     break;

    // }

    if(receivedEventBase->signal == AppDeneme1::AppSignals::SIGNAL_1){
        std::cout << "this is task2 - event: " << ++_count << std::endl;
        
        auto event1 = uOS::recast_e<AppDeneme1::Event1>(receivedEventBase);
        std::cout << "_stateMachine after_cast - event copy count = " << receivedEventBase.use_count() << std::endl;
        std::cout << event1->s << std::endl;

        if(_count == 10)
            unsubscribe(AppDeneme1::AppSignals::SIGNAL_1);
    }
}

void Task2::_init(void)
{
    subscribe(AppDeneme1::AppSignals::SIGNAL_1);
    // subscribe(AppDeneme1::AppSignals::SIGNAL_3);

    // auto event = uOS::new_e<uOS::Event>(Task2signals::PUBLISH);
    // uOS::FW::postEventIn(22000, _taskId, event);
}
