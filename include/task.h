#ifndef TASK_H
#define TASK_H

#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>

#include "logger.h"
//#include "uOS_defs.h"
#include "state_machine.h"


namespace uOS {


class Task : public StateMachine
{
    public:
        virtual ~Task();

        void start(void);
        void stop(void);
        void resume(void);
        void putEvent(std::shared_ptr<const Event>&&);
        //void deleteTask(void);  // delete is keyword -- destructor ile yapabiliriz

    protected:
        Task(const TaskId, std::string&&);
        Task() = delete;
        Task(const Task& other) = delete;
        Task& operator= (const Task& rhs) = delete;

        void subscribe(SignalId);
        void unsubscribe(SignalId);
        void putEvent(std::shared_ptr<const Event>&);   // todo: bunun adı selfPost benzeri birşey olacak

        const TaskId _id;
        const std::string _name;
        
        template <class T = Event>
        T* new_e(std::optional<SignalId> signalId = std::nullopt)
        {
            T* e = uOS::new_e<T>(signalId);
            e->source = _id;
            return e;
        }

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
            if(Logger::isActive() == false) {
                return;
            }
            // format logStr
            auto formattedStr = fmt::format(logStr, logStrArgs...);

            // sent to logger
            Logger::logTask(file, function, line, level, formattedStr, _id);
        } 
};


}   // namespace uOS


#endif  // TASK_H
