#ifndef TASK_H
#define TASK_H

#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>

#include "uOS_defs.h"
#include "state_machine.h"
#include "logger.h"


namespace uOS {


class Task : public StateMachine
{
    public:
        virtual ~Task() {};

        void start(void);
        void putEvent(std::shared_ptr<const Event>&&);
        void stop(void);
        void resume(void);
        //void deleteTask(void);  // delete is keyword -- destructor ile yapabiliriz

    protected:
        Task(const TaskId, std::string&&);
        Task() = delete;
        Task(const Task& other) = delete;
        Task& operator= (const Task& rhs) = delete;

        void subscribe(SignalId);
        void unsubscribe(SignalId);
        void putEvent(std::shared_ptr<const Event>&);

        const TaskId _taskId;
        const std::string _name;
        const std::string _logPrefix;

    private:
        void _taskLoop(void);
        std::shared_ptr<const Event> _getEvent(void);

        std::queue<std::shared_ptr<const Event>> _eventQueue;
        std::mutex _queueMutex;

        volatile bool _isRunning = false;
        volatile bool _isUnlocked = true;
        std::thread _taskThead;
        std::mutex _threadMutex;
        std::condition_variable _taskCv;

    protected:
        template <class... LogStrArgs>
        void log__(const char* file, const char* function, int line, LogLevel level, const std::string& logStr, LogStrArgs&... logStrArgs) {
            // format logStr
            auto formattedStr = fmt::format(logStr, logStrArgs...);

            //add task prefix
            formattedStr.insert(0, _logPrefix);

            // sent to logger
            Logger::log(file, function, line, level, formattedStr);
        } 
};


}   // namespace uOS

#endif
