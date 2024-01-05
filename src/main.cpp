
#include <iostream>
#include "uOS.h"
#include "App1.h"
#include "task1.h"
// #include "task2.h"
// #include "task3.h"

uOS::TaskId g_task1Id, g_task2Id, g_task3Id;

void postEvt(void){
    auto event = uOS::new_e<uOS::Event>(App1::AppSignals::TRANS_TO_2);  // default template oluyor mu? :) base event için
    auto event2 = uOS::new_e<uOS::Event>(App1::AppSignals::TRANS_TO_1);

    // std::cout << "main first copy count = " << event.use_count() << std::endl;
    //std::cout << "main before - event copy count = " << event.use_count() << std::endl;
    uOS::FW::postEventIn(3000, g_task1Id, event);
    uOS::FW::postEventIn(6000, g_task1Id, event2);

    //std::cout << "main after - event copy count = " << event.use_count() << std::endl;
    //std::cout << "main second copy count = " << event.use_count() << std::endl;
    //uOS::FW::publishEventEvery(5000, event);

}

int main(int argc, char* argv[])
{
    std::cout << "hello world" << std::endl;

    uOS::FW::init();

    g_task1Id = uOS::FW::createTask<App1::Task1>("Task-1", 33, 33.3, "otuz-uc");
    // g_task2Id = uOS::FW::createTask<Task2>("Task-2");
    // g_task3Id = uOS::FW::createTask<Task3>("Task-3");

    uOS::FW::startTask(g_task1Id);
    // uOS::FW::startTask(g_task2Id);
    // uOS::FW::startTask(g_task3Id);

    postEvt();

    return uOS::FW::run();
}


// task içerisinde put eventi move demeden çağırınca gerçekten protected olan mı çağırılıyor?
// put event içirisinde emplace edilmediğinde (şuandaki halinde) nullptr oluyor mu bakılcak
// olmuyorsa emplace içerisinde move edilecek