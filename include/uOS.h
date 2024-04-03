#ifndef uOS_H
#define uOS_H

#include <cassert>
#include <type_traits>

#include <utility>
#include <functional>
#include <optional>
#include <chrono>

#include <array>
#include <vector>
#include <forward_list>
#include <unordered_map>

#include "uOS_defs.h"
#include "task.h"

#if uOS_FW_MAX_TIMER_COUNT > 65536
using TimerId = uint_fast32_t;
#else
using TimerId = uint_fast16_t;
#endif


namespace uOS {


// static class, wont get instatiated
class FW
{
    public:
        // fw op
        static void init(int = 0, char*[] = nullptr);
        static int32_t run(void);
        static void stop(void);

        // task op
        static void startTask(TaskId);
        static void stopTask(TaskId);
        static bool resumeTask(TaskId);
        static bool deleteTask(TaskId);

        // event delivery 
        static void postEvent(TaskId, const Event*);
        static void publishEvent(const Event*);

        static TimerId postEventIn(uint_fast16_t intervalMs, TaskId, const Event*);
        static TimerId postEventEvery(uint_fast16_t intervalMs, TaskId, const Event*);
        static TimerId publishEventIn(uint_fast16_t intervalMs, const Event*);
        static TimerId publishEventEvery(uint_fast16_t intervalMs, const Event*);
        static void cancelTimedEvent(TimerId);

        // pub/sub

    private:
        // private definitions
        struct TimerNode {
            bool isPublish;
            bool isPeriodic;
            TaskId receiverTaskId;
            uint_fast16_t interval;
            std::shared_ptr<const Event> eventPtr;
        };

        static constexpr uint_fast16_t uOS_FW_MAX_TIMER_COUNT = 2048;

        static volatile inline bool _isRunning = true;

        // task related
        static inline std::array<bool, uOS_FW_MAX_TASK_COUNT> _taskIdUsageList;
        static inline std::unordered_map<TaskId, std::unique_ptr<Task>> _taskMap;
        static inline std::mutex _taskMutex;

        // timer related
        static inline std::array<bool, uOS_FW_MAX_TIMER_COUNT> _timerIdUsageList;
        static inline std::unordered_map<TimerId, TimerNode> _timerStorageMap;
        static inline std::forward_list<std::pair<TimerId, uint_fast16_t>> _timerQueue;
        static inline std::mutex _timerMutex;

        // pub-sub related
        static inline std::unordered_map<SignalId, std::vector<TaskId>> _subsMap;
        static inline std::mutex _subsMutex;

        // private functions
        static TaskId _getAvailableTaskId(void);
        static TimerId _getAvailableTimerId(void);
        static void _insertTimerQueue(TimerId, uint_fast16_t);
        static TimerId _setupTimedEvent(bool, bool, uint_fast16_t, const Event*, TaskId receiverTaskId = 0);
        static void _handleTick(void);
        static void _publishEvent(std::shared_ptr<const Event>&);
        static void _publishEventCommon(std::shared_ptr<const Event>&, std::vector<TaskId>&);

        static void subscribe(SignalId, TaskId);    // todo: bunlar public olacak, task class'ından friend function olarak işaretlenecek
        static void unsubscribe(SignalId, TaskId);  // Task* parametresi alacak, task'dan this ile çağırılacak. "friend class Task;" -> bu kaldırılacak

        friend class Task;

    public:
        template <class T, class... TaskArgs>
        static TaskId createTask(std::string&& taskName = "", const TaskArgs&... taskArgs)
        {   
            static_assert(std::is_base_of_v<Task, T>);
            std::lock_guard<std::mutex> lock(_taskMutex);

            TaskId newTaskId = FW::_getAvailableTaskId();
            
            const auto [_, result] = _taskMap.emplace(newTaskId, std::make_unique<T>(newTaskId, std::move(taskName), taskArgs...));
            assert(result == true);

            return newTaskId;
        }
};

// log view finalize
// file logger
// cli??
// cmake with examples (add final, fmt, etc. as modules)

// bitset kullanılabilecek yerler var mı?
// custom objelerin operatörlerini overload edebileceğimiz durumlar var mı(bool, &&, >, < vs.)


}   // namespace uOS


#endif  // uOS_H
