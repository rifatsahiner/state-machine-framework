#ifndef uOS_DEFS_H
#define uOS_DEFS_H

#include <cstdint>
#include <memory>
#include <optional>

static constexpr uint_fast16_t uOS_FW_MAX_TASK_COUNT = 256; 


namespace uOS {


// taskId
#if uOS_FW_MAX_TASK_COUNT > 256
using TaskId = uint_fast16_t;
#else
using TaskId = uint_fast8_t;
#endif

// signal
using SignalId = uint_fast16_t;

// published tick signals
constexpr SignalId SIG_TICK_SEC = static_cast<SignalId>(4);
constexpr SignalId SIG_TICK_MIN = static_cast<SignalId>(5);
constexpr SignalId SIG_TICK_HOUR = static_cast<SignalId>(6);

/// Offsets for user signals
constexpr SignalId INT_SIGNAL_START = static_cast<SignalId>(7);
constexpr SignalId EXT_SIGNAL_START = static_cast<SignalId>(32);

// event
struct Event {
    SignalId signal;
    std::optional<TaskId> source;
};

// event utility
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

// log levels
enum class LogLevel : uint_fast8_t {
    LOG_TRACE = 0,
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR
};

// log macro
#define LOG(logLevel, logStr, ...) log__(__FILE__, __func__, __LINE__, logLevel, logStr, ##__VA_ARGS__)


}   // namespace uOS


#endif  // uOS_DEFS_H
