
#include <string>

#include "uOS.h"
#include "App1.h"
#include "simple-task.h"

int main(int argc, char* argv[])
{
    if(argc == 1) {
        uOS::FW::init(argc, argv);
    } else {
        for (int i = 1; i < argc; ++i) {
            if(std::string{"--no-tui"}.compare(argv[i]) == 0){
                uOS::FW::init();
            } else {
                exit(2);
            }
        }
    }

    // create test task
    App1::g_simpleTaskId = uOS::FW::createTask<App1::SimpleTask>("simple-task");

    // start test task
    uOS::FW::startTask(App1::g_simpleTaskId);

    // run fw
    exit(uOS::FW::run());
}


// task içerisinde put eventi move demeden çağırınca gerçekten protected olan mı çağırılıyor?
// put event içirisinde emplace edilmediğinde (şuandaki halinde) nullptr oluyor mu bakılcak
// olmuyorsa emplace içerisinde move edilecek 