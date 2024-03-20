
#include "App1.h"
#include "simple-task.h"

// internal signals
enum TaskSignals : uOS::SignalId
{
    SIGNAL_LOG = uOS::INT_SIGNAL_START
};


namespace App1 {


SimpleTask::SimpleTask(const uOS::TaskId taskId, std::string&& taskName) : Task (taskId, std::move(taskName)) {}

uOS::HandleResult SimpleTask::_topInitialTrans(void){
    auto event = uOS::new_e(SIGNAL_LOG);
    uOS::FW::postEventEvery(2000, _taskId, event);

    //std::cout << "top-INIT;";
    LOG(uOS::LogLevel::LOG_INFO, "top-INIT");
    return trans(&SimpleTask::singleState);
}

uOS::HandleResult SimpleTask::singleState(const std::shared_ptr<const uOS::Event>& event){
    uOS::HandleResult res;

    switch (event->signal) {
        case SIGNAL_LOG: {
            LOG(uOS::LogLevel::LOG_INFO, "simple task log {}", _counter);
            _counter++;
            res = handled();
            break;
        }
        default: {
            res = super(&SimpleTask::top);
            break;
        }
    }
    return res;
}


}   // namespace App1
