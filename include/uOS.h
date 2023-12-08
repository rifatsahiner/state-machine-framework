#ifndef uOS_H
#define uOS_H

#include <cstdint>
#include <cassert>
#include <type_traits>

#include <memory>
#include <utility>
#include <functional>

#include <thread>
#include <mutex>
#include <condition_variable>

#include <array>
#include <vector>
#include <queue>
#include <forward_list>
#include <unordered_map>


namespace uOS {


static constexpr uint_fast16_t uOS_FW_MAX_TASK_COUNT = 256;
static constexpr uint_fast16_t uOS_FW_MAX_TIMER_COUNT = 2048;

#if uOS_FW_MAX_TASK_COUNT > 256
using TaskId = uint_fast16_t;
#else
using TaskId = uint_fast8_t;
#endif

#if uOS_FW_MAX_TASK_COUNT > 65536
using TimerId = uint_fast32_t;
#else
using TimerId = uint_fast16_t;
#endif

using SignalId = uint_fast16_t;
static constexpr SignalId EXT_SIGNAL_START = 64;

//////////////////////////////////////////////////////
//                                                  //
//////////////////////////////////////////////////////

// base task IDs
extern TaskId g_task1Id, g_task2Id, g_task3Id;      // todo: bu buradan kaldırılmalı

//////////////////////////////////////////////////////
//                                                  //
//////////////////////////////////////////////////////

struct Event {
    SignalId signal;
};

struct TimerNode {
    bool isPublish;
    bool isPeriodic;
    TaskId receiverTaskId;
    uint_fast16_t interval;
    std::shared_ptr<const Event> eventPtr;

    // default move constructor kullanıldığını belirt
    // TimerNode(TimerNode&& other) = default;
};

template <class T>
T* new_e(SignalId signalId)
{
    static_assert(std::is_base_of_v<Event, T>);
    
    T* newEventPtr = new T; 
    newEventPtr->signal = signalId;
    return newEventPtr;
}

template <class T>
const T* recast_e(std::shared_ptr<const Event>& eventSptr){
    static_assert(std::is_base_of_v<Event, T>);
    return static_cast<const T*>(eventSptr.get());
}

//////////////////////////////////////////////////////
//                                                  //
//////////////////////////////////////////////////////

class Task {
    public:
        Task() = delete;
        Task(const TaskId, const std::string&);
        Task(const Task& other) = delete;
        Task& operator= (const Task& rhs) = delete;
        
        void start(void);   // bunlar private yapılıp FW friend class yapılarak çağırtılabilir
        void putEvent(std::shared_ptr<const Event>&&);
        void stop(void);
        void resume(void);

    protected:
        const TaskId _taskId;
        const std::string _name;
        
        //Task() = delete;  <-bu şekilde protected içerisinde oluyor mu?
        //Task(TaskId);
        void subscribe(SignalId);
        void unsubscribe(SignalId);
        void putEvent(std::shared_ptr<const Event>&);

    private:
        void _taskLoop(void);
        std::shared_ptr<const Event> _getEvent(void);
        virtual void _stateMachine(std::shared_ptr<const Event>&) = 0;
        virtual void _init(void) = 0;
        // todo: hsm tasarımına göre eğer hiç virtual func. olmazsa abstarct'lık constructor protected'a alınarak yapılsın
        // bu sayede v-table'lar ile zaman kaybedilmemiş olur

        std::function<void(std::shared_ptr<const Event>)> _currentState;
        volatile bool _isRunning = false;
        volatile bool _isUnlocked = true;
        std::thread _taskThead;
        std::queue<std::shared_ptr<const Event>> _eventQueue;
        std::mutex _queueMutex;
        std::condition_variable _taskCv;
        std::mutex _threadMutex;
};

//////////////////////////////////////////////////////
//                                                  //
//////////////////////////////////////////////////////

// static class, wont get instatiated
class FW {
    public:
        static void init(void);
        static int32_t run(void);

        static void startTask(TaskId);
        static bool stopTask(TaskId);
        static bool deleteTask(TaskId);
        static bool resumeTask(TaskId);

        static void postEvent(TaskId, const Event*);
        static TimerId postEventIn(uint_fast16_t intervalMs, TaskId, const Event*);
        static TimerId postEventEvery(uint_fast16_t intervalMs, TaskId, const Event*);
        static void cancelTimedEvent(TimerId);

        static void publishEvent(const Event*);
        static TimerId publishEventIn(uint_fast16_t intervalMs, const Event*);
        static TimerId publishEventEvery(uint_fast16_t intervalMs, const Event*);

    private:
        static volatile inline bool _isRunning = true;

        static inline std::array<bool, uOS_FW_MAX_TASK_COUNT> _taskIdUsageList;
        static inline std::unordered_map<TaskId, std::unique_ptr<Task>> _taskMap;
        static inline std::mutex _taskMutex;

        static inline std::array<bool, uOS_FW_MAX_TIMER_COUNT> _timerIdUsageList;
        static inline std::unordered_map<TimerId, TimerNode> _timerStorageMap;
        static inline std::forward_list<std::pair<TimerId, uint_fast16_t>> _timerQueue;
        static inline std::mutex _timerMutex;

        static inline std::unordered_map<SignalId, std::vector<TaskId>> _subsMap;
        static inline std::mutex _subsMutex;

        static TaskId _getAvailableTaskId(void);
        static TimerId _getAvailableTimerId(void);
        static void _insertTimerQueue(TimerId, uint_fast16_t);
        static TimerId _setupTimedEvent(bool, bool, uint_fast16_t, const Event*, TaskId receiverTaskId = 0);
        static void _handleTick(void);
        static void _publishEvent(std::shared_ptr<const Event>&);
        static void _publishEventCommon(std::shared_ptr<const Event>&, std::vector<TaskId>&);

        static void subscribe(SignalId, TaskId);
        static void unsubscribe(SignalId, TaskId);

        friend class Task;

    public:
        template <class T>
        static TaskId createTask(const std::string& taskName)      // todo: sabit (static) tasklar için taskId manual verebildiğin 2. bir versiyonu olacak
        {
            static_assert(std::is_base_of_v<Task, T>);
            std::lock_guard<std::mutex> lock(_taskMutex);

            TaskId newTaskId = FW::_getAvailableTaskId();
            _taskMap.emplace(newTaskId, std::make_unique<T>(newTaskId, taskName));
            // todo: assert if emplace fails

            return newTaskId;
        }

        
};

// event queue  -> std::deque with std::lock_guard
// timer
// pub-sub
// hsm
    // bitset kullanılabilecek yerler var mı?
    // custom objelerin operatörlerini overload edebileceğimiz durumlar var mı(bool, &&, >, < vs.)
// command-line interface + trace (**TUI**)- nasıl olmalı, nasıl implemente edilecek
// file logger ??


}   // namespace uOS

#endif
