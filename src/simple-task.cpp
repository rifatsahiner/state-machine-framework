
#include "App1.h"
#include "simple-task.h"

// internal signals
enum TaskSignals : SignalId
{
    SIGNAL_LOG = INT_SIGNAL_START
};


namespace App1 {


SimpleTask::SimpleTask(const TaskId taskId, std::string&& taskName) : Task (taskId, std::move(taskName)) {}

HandleResult SimpleTask::_topInitialTrans(void){
    auto event = new_e(SIGNAL_LOG);
    FW::postEventEvery(3000, _id, event);

    subscribe(SIG_TICK_SEC);
    subscribe(SIG_TICK_MIN);
    subscribe(SIG_TICK_HOUR);

    LOG(LogLevel::LOG_INFO, "top-INIT");
    return trans(&SimpleTask::singleState);
}

HandleResult SimpleTask::singleState(const std::shared_ptr<const Event>& event){
    HandleResult res;

    switch (event->signal) {
        case SIGNAL_LOG: {
            LOG(LogLevel::LOG_INFO, "simple task 3-second log {}", _counter);
            _counter++;
            res = handled();
            break;
        }
        case SIG_TICK_SEC: {
            LOG(LogLevel::LOG_INFO, "simple task second-tick log");
            res = handled();
            break;
        }
        case SIG_TICK_MIN: {
            LOG(LogLevel::LOG_INFO, "simple task minute-tick log");
            res = handled();
            break;
        }
        case SIG_TICK_HOUR: {
            LOG(LogLevel::LOG_INFO, "simple task hour-tick log");
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
