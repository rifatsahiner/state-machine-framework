
#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <thread>

#include "uOS_defs.h"
#include "fmt/core.h"

class FLogView;


namespace uOS {


class Logger {
    public:
        static void init(int, char*[]);
        static void start(void);
        static void stop(void);
        static void log(const char*, const char*, int, LogLevel, std::string&);
    
    private:
        static inline FLogView* _loggerHandle;
        static inline std::thread _loggerThread;
};

template <class... LogStrArgs>
void log__(const char* file, const char* function, int line, LogLevel level, const std::string& logStr, LogStrArgs&... logStrArgs) {
    // format logStr
    auto formattedStr = fmt::format(logStr, logStrArgs...);

    // sent to logger
    Logger::log(file, function, line, level, formattedStr);
}


}   // namesapce uOS

#endif
