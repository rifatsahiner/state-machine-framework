
#include "logger.h"
#include "flogview.h"

void tuiThreadFunc(void) {
    finalcut::FApplication::getApplicationObject()->exec();
}

namespace uOS {


void Logger::init(int argc, char* argv[], std::function<void(void)> quitCb)
{
    //static finalcut::FApplication app(argc, argv);
    _tui = new finalcut::FApplication(argc, argv);

    // Force terminal initialization without calling show()
    //app.initTerminal();
    _tui->initTerminal();

    // create logger
    //_loggerFw = new FLogView(_tui, 2000);
    _loggerTasks = new FLogViewMulti(_tui, 2000);
    finalcut::FWidget::setMainWidget(_loggerTasks);

    // configure loger
    _loggerTasks->setText(L"Logger");
    _loggerTasks->unsetShadow();
    _loggerTasks->setResizeable(true);
    _loggerTasks->setGeometry(finalcut::FPoint{1,1}, finalcut::FSize{_tui->getDesktopWidth(), _tui->getDesktopHeight()});
    _loggerTasks->registerOnQuit(quitCb);
    _loggerTasks->show();  // todo: bunu start'a taşıyabiliriz
}

void Logger::start(void) {
    if(_tui != nullptr){
        _loggerThread = std::thread{tuiThreadFunc};
    }
}

void Logger::stop(void) {
    _tui->quit();
    _loggerThread.join();
    delete _tui;
    _tui = nullptr;
}

void Logger::addTaskLogger(const TaskId taskId, const std::string& taskName) {
    if(_tui != nullptr){
        _loggerTasks->createView(static_cast<uint_fast16_t>(taskId), taskName);
    }
}

void Logger::removeTaskLogger(const TaskId taskId) {
    _loggerTasks->removeView(static_cast<uint_fast16_t>(taskId));
}

void Logger::logFw(const char* file, const char* function, int line, LogLevel level, std::string& formattedLogStr) {
    //_logCommon(file, function, line, level, formattedLogStr);
}

void Logger::logTask(const char* file, const char* function, int line, LogLevel level, std::string& formattedLogStr, TaskId taskId) {
    _logCommon(file, function, line, level, formattedLogStr, taskId);
}

bool Logger::isActive(void) {
    if(_tui == nullptr){
        return false;
    } else {
        return true;
    }
}

//////////////////////////////////////////////////////
//                                                  //
//////////////////////////////////////////////////////

inline void Logger::_logCommon(const char* file, const char* function, int line, LogLevel level, std::string& logStr, std::optional<TaskId> taskId) {
    // Get the current time and convert to time struct
    auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::tm* timeStruct = std::localtime(&now);

    // "HH:MM:SS FILE:LINE:FUNC [|] logstr"
    // add space and function
    logStr.insert(0, " [|] ");
    logStr.insert(0, function);
    logStr.insert(0, 1, ':');
    // add line
    logStr.insert(0, std::to_string(line));
    logStr.insert(0, 1, ':');
    // add file
    // remove file path
    logStr.insert(0, file);
    auto pos = logStr.find_first_of('/');
    if(pos != std::string::npos){
        logStr.erase(0, pos+1);
    }
    logStr.insert(0, 1, ' ');
    //add sec
    logStr.insert(0, std::to_string(timeStruct->tm_sec));
    if(timeStruct->tm_sec < 10)
        logStr.insert(0, ":0");
    else 
        logStr.insert(0, 1, ':');
    // add min
    logStr.insert(0, std::to_string(timeStruct->tm_min));
    if(timeStruct->tm_min < 10)
        logStr.insert(0, ":0");
    else 
        logStr.insert(0, 1, ':');
    // add hour
    logStr.insert(0, std::to_string(timeStruct->tm_hour));
    if(timeStruct->tm_hour < 10)
        logStr.insert(0, 1, '0');

    // convert string to wide string
    auto src = logStr.c_str();
    auto state = std::mbstate_t();
    auto size = std::mbsrtowcs(nullptr, &src, 0, &state) + 1;

    std::vector<wchar_t> dest(size);
    const auto wide_length = std::mbsrtowcs(dest.data(), &src, size, &state);

    // print to log view
    if(taskId){
        _loggerTasks->log(std::wstring{dest.data(), wide_length}, static_cast<FLogView::LogLevel>(level), static_cast<uint16_t>(taskId.value()));
    } else {
        _loggerFw->log(std::wstring{dest.data(), wide_length}, static_cast<FLogView::LogLevel>(level));
    }
}


}   // namespace uOS
