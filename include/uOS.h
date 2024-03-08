#ifndef uOS_H
#define uOS_H

#include <cassert>
#include <type_traits>

#include <utility>
#include <functional>
#include <optional>

#include <array>
#include <vector>
#include <forward_list>
#include <unordered_map>

#include "task.h"


namespace uOS {


constexpr uint_fast16_t uOS_FW_MAX_TIMER_COUNT = 2048;

#if uOS_FW_MAX_TIMER_COUNT > 65536
using TimerId = uint_fast32_t;
#else
using TimerId = uint_fast16_t;
#endif

/// Offsets for user signals
constexpr SignalId INT_SIGNAL_START = static_cast<SignalId>(4);
constexpr SignalId EXT_SIGNAL_START = static_cast<SignalId>(32);

//////////////////////////////////////////////////////
//                                                  //
//////////////////////////////////////////////////////

struct TimerNode {
    bool isPublish;
    bool isPeriodic;
    TaskId receiverTaskId;
    uint_fast16_t interval;
    std::shared_ptr<const Event> eventPtr;

    // default move constructor kullanıldığını belirt
    // TimerNode(TimerNode&& other) = default;
};

template <class T = Event>
T* new_e(std::optional<SignalId> signalId = std::nullopt)
{
    static_assert(std::is_base_of_v<Event, T>);
    
    T* newEventPtr = new T; 
    if(signalId){
        newEventPtr->signal = signalId.value();
    }
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

// static class, wont get instatiated
class FW
{
    public:
        static void init(void);
        static int32_t run(void);
        static void stop(void);

        static void startTask(TaskId);
        static void stopTask(TaskId);
        static bool resumeTask(TaskId);
        static bool deleteTask(TaskId);

        static void postEvent(TaskId, const Event*);
        static void publishEvent(const Event*);

        static TimerId postEventIn(uint_fast16_t intervalMs, TaskId, const Event*);
        static TimerId postEventEvery(uint_fast16_t intervalMs, TaskId, const Event*);
        static TimerId publishEventIn(uint_fast16_t intervalMs, const Event*);
        static TimerId publishEventEvery(uint_fast16_t intervalMs, const Event*);
        static void cancelTimedEvent(TimerId);

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
        template <class T, class... TaskArgs>
        static TaskId createTask(const std::string& taskName = "", const TaskArgs&... taskArgs)
        {   
            static_assert(std::is_base_of_v<Task, T>);
            std::lock_guard<std::mutex> lock(_taskMutex);

            TaskId newTaskId = FW::_getAvailableTaskId();
            
            const auto [_, result] = _taskMap.emplace(newTaskId, std::make_unique<T>(newTaskId, taskName, taskArgs...));
            assert(result == true);

            return newTaskId;
        }   
};

// event queue  -> std::deque with std::lock_guard
// timer
// pub-sub
// hsm
// her taskın kendine özel logger'ı olacak
// hazır cli ?
// examples -> make ile sadece ilgili bir example build et denecek
    // bitset kullanılabilecek yerler var mı?
    // custom objelerin operatörlerini overload edebileceğimiz durumlar var mı(bool, &&, >, < vs.)
// command-line interface + trace (**TUI**)- nasıl olmalı, nasıl implemente edilecek
// file logger ??


}   // namespace uOS


#endif
