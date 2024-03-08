
#include <iostream>
#include "uOS.h"
#include "App1.h"
#include "hsm_test.h"
#include "key_reader.h"

int main(int argc, char* argv[])
{
    // int32_t r;
    (void)argc;
    (void)argv;

    std::cout << "HSM test example\n";
    std::cout << "Press ESC to quit...\n";

    uOS::FW::init();

    // create test task
    App1::g_hsmTestId = uOS::FW::createTask<App1::HsmTest>("HSM-test");

    // start test task
    uOS::FW::startTask(App1::g_hsmTestId);

    // create keyboard reader task (no FW)
    std::thread t(App1::keyReadThread);
    t.detach();

    exit(uOS::FW::run());
}


// task içerisinde put eventi move demeden çağırınca gerçekten protected olan mı çağırılıyor?
// put event içirisinde emplace edilmediğinde (şuandaki halinde) nullptr oluyor mu bakılcak
// olmuyorsa emplace içerisinde move edilecek