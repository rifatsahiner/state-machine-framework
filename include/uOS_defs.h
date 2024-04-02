#ifndef uOS_DEFS_H
#define uOS_DEFS_H

#include <cstdint>

static constexpr uint_fast16_t uOS_FW_MAX_TASK_COUNT = 256; 


namespace uOS {

// taskId
#if uOS_FW_MAX_TASK_COUNT > 256
using TaskId = uint_fast16_t;
#else
using TaskId = uint_fast8_t;
#endif

// signal id
using SignalId = uint_fast16_t;

/// Offsets for user signals
constexpr SignalId INT_SIGNAL_START = static_cast<SignalId>(4);
constexpr SignalId EXT_SIGNAL_START = static_cast<SignalId>(32);

// event
struct Event {
    SignalId signal;    // todo acil: optional senderId ekelenecek
};

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


#endif
