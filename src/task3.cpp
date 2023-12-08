
#include "deneme1.h"
#include "task3.h"
#include <iostream>

//extern uOS::TaskId g_task2Id;

// internal signals
enum Task3signals : uOS::SignalId {
    PUBLISH,
    SIGNAL_LIST_COUNT
};

Task3::Task3(const uOS::TaskId taskId, const std::string& taskName) : Task(taskId, taskName) { }

void Task3::_stateMachine(std::shared_ptr<const uOS::Event>& receivedEventBase) 
{
    // switch(receivedEventBase->signal) 
    // {
    //     case Task3signals::PUBLISH: {
    //         auto event = uOS::new_e<AppDeneme1::Event1>(AppDeneme1::AppSignals::SIGNAL_3);
    //         event->s = "this is event from task 3\n";
    //         uOS::FW::publishEvent(event); }
    //     break;
        
    //     case AppDeneme1::AppSignals::SIGNAL_1:
    //     case AppDeneme1::AppSignals::SIGNAL_2:
    //         auto event1 = uOS::recast_e<AppDeneme1::Event1>(receivedEventBase);
    //         std::cout << event1->s << std::endl;
    //     break;

    // }

    if(receivedEventBase->signal == AppDeneme1::AppSignals::SIGNAL_1){
        std::cout << "this is task3 - event: " << ++_count << std::endl;
        
        auto event1 = uOS::recast_e<AppDeneme1::Event1>(receivedEventBase);
        std::cout << "_stateMachine after_cast - event copy count = " << receivedEventBase.use_count() << std::endl;
        std::cout << event1->s << std::endl;

        if(_count == 15)
            unsubscribe(AppDeneme1::AppSignals::SIGNAL_1);
    }
}

void Task3::_init(void)
{
    subscribe(AppDeneme1::AppSignals::SIGNAL_1);
    // subscribe(AppDeneme1::AppSignals::SIGNAL_2);

    // auto event = uOS::new_e<uOS::Event>(Task3signals::PUBLISH);
    // uOS::FW::postEventIn(33000, _taskId, event);
}
