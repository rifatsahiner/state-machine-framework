#ifndef TASK_H
#define TASK_H

#include <thread>
#include <mutex>
#include <condition_variable>

#include <queue>

#include "state_machine.h"


namespace uOS {


constexpr uint_fast16_t uOS_FW_MAX_TASK_COUNT = 256;         // todo: bunlar uos_config.h dosyasına taşınacak

#if uOS_FW_MAX_TASK_COUNT > 256
using TaskId = uint_fast16_t;
#else
using TaskId = uint_fast8_t;
#endif

//////////////////////////////////////////////////////
//                                                  //
//////////////////////////////////////////////////////

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
        Task(const TaskId, const std::string&);
        Task() = delete;
        Task(const Task& other) = delete;
        Task& operator= (const Task& rhs) = delete;

        void subscribe(SignalId);
        void unsubscribe(SignalId);
        void putEvent(std::shared_ptr<const Event>&);

        const TaskId _taskId;
        const std::string _name;

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
};

}   // namespace uOS


#endif