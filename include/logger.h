
#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <thread>
#include <functional>

#include "uOS_defs.h"
#include "fmt/core.h"

class FLogView;
class FLogViewMulti;
namespace finalcut { class FApplication; }


namespace uOS {


class Logger {
    public:
        static void init(int, char*[], std::function<void(void)>);
        static void start(void);
        static void stop(void);
        static void addTaskLogger(const TaskId, const std::string&);
        static void removeTaskLogger(const TaskId);
        static void logFw(const char*, const char*, int, LogLevel, std::string&);
        static void logTask(const char*, const char*, int, LogLevel, TaskId, std::string&);
        static bool isActive(void);
    
    private:
        static inline finalcut::FApplication* _tui {nullptr};
        static inline FLogView* _loggerFw;
        static inline FLogViewMulti* _loggerTasks;
        static inline std::thread _loggerThread;

        static inline void _logCommon(const char*, const char*, int, LogLevel, std::string&, std::optional<TaskId> = std::nullopt);
};

template <class... LogStrArgs>
void log__(const char* file, const char* function, int line, LogLevel level, const std::string& logStr, LogStrArgs&... logStrArgs) {
    if(Logger::isActive() == false){
        return;
    }
    
    // format logStr
    auto formattedStr = fmt::format(logStr, logStrArgs...);     //std::forward olmuyor mu?

    // sent to logger
    Logger::logFw(file, function, line, level, formattedStr);
}


}   // namesapce uOS

#endif
