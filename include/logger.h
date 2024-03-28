
#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <thread>
#include <functional>

#include "uOS_defs.h"
#include "fmt/core.h"

class FLogView;
namespace finalcut { class FApplication; }


namespace uOS {


class Logger {
    public:
        static void init(int, char*[], std::function<void(void)>);
        static void start(void);
        static void stop(void);
        static void log(const char*, const char*, int, LogLevel, std::string&);
    
    private:
        //static inline FLogView* _loggerHandle;
        static inline finalcut::FApplication* _appPtr;
        static inline FLogView* _logViewPtr;

        static inline std::thread _loggerThread;
};

template <class... LogStrArgs>
void log__(const char* file, const char* function, int line, LogLevel level, const std::string& logStr, LogStrArgs&... logStrArgs) {
    // format logStr
    auto formattedStr = fmt::format(logStr, logStrArgs...);     //std::forward olmuyor mu?

    // sent to logger
    Logger::log(file, function, line, level, formattedStr);
}


}   // namesapce uOS

#endif
