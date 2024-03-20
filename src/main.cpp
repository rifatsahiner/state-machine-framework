
#include "uOS.h"
#include "App1.h"
#include "simple-task.h"

int main(int argc, char* argv[])
{
    // std::cout << "HSM test example\n";
    // std::cout << "Press ESC to quit...\n";

    uOS::FW::init(argc, argv);

    // create test task
    App1::g_simpleTaskId = uOS::FW::createTask<App1::SimpleTask>("simple-task");

    // start test task
    uOS::FW::startTask(App1::g_simpleTaskId);

    exit(uOS::FW::run());
}


// task içerisinde put eventi move demeden çağırınca gerçekten protected olan mı çağırılıyor?
// put event içirisinde emplace edilmediğinde (şuandaki halinde) nullptr oluyor mu bakılcak
// olmuyorsa emplace içerisinde move edilecek