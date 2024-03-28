
#include "logger.h"
#include "flogview.h"

void tuiThreadFunc(void) {
    finalcut::FApplication::getApplicationObject()->exec();
}

namespace uOS {


void Logger::init(int argc, char* argv[], std::function<void(void)> quitCb)
{
    //static finalcut::FApplication app(argc, argv);
    _appPtr = new finalcut::FApplication(argc, argv);

    // Force terminal initialization without calling show()
    //app.initTerminal();
    _appPtr->initTerminal();

    // create logger
    //static FLogView logger(&app, 1000);
    //_loggerHandle = &logger;
    _logViewPtr = new FLogView(_appPtr, 1000);
    finalcut::FWidget::setMainWidget(_logViewPtr);

    // configure loger
    _logViewPtr->setText(L"Logger");
    _logViewPtr->unsetShadow();
    _logViewPtr->setResizeable(true);
    _logViewPtr->setGeometry(finalcut::FPoint{1,1}, finalcut::FSize{_appPtr->getDesktopWidth(), _appPtr->getDesktopHeight()});
    _logViewPtr->registerOnQuit(quitCb);
    _logViewPtr->show();  // todo: bunu start'a taşıyabiliriz
}

void Logger::start(void) {
    _loggerThread = std::thread{tuiThreadFunc};
}

void Logger::stop(void) {
    finalcut::FApplication::getApplicationObject()->quit();
    _loggerThread.join();
    delete _appPtr;
}

void Logger::log(const char* file, const char* function, int line, LogLevel level, std::string& formattedLogStr) {
    // Get the current time and convert to time struct
    auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::tm* timeStruct = std::localtime(&now);

    // "HH:MM:SS FILE:LINE:FUNC [|] logstr"
    // add space and function
    formattedLogStr.insert(0, " [|] ");
    formattedLogStr.insert(0, function);
    formattedLogStr.insert(0, 1, ':');
    // add line
    formattedLogStr.insert(0, std::to_string(line));
    formattedLogStr.insert(0, 1, ':');
    // add file
    // remove file path
    formattedLogStr.insert(0, file);
    auto pos = formattedLogStr.find_first_of('/');
    if(pos != std::string::npos){
        formattedLogStr.erase(0, pos+1);
    }
    formattedLogStr.insert(0, 1, ' ');
    //add sec
    formattedLogStr.insert(0, std::to_string(timeStruct->tm_sec));
    if(timeStruct->tm_sec < 10)
        formattedLogStr.insert(0, ":0");
    else 
        formattedLogStr.insert(0, 1, ':');
    // add min
    formattedLogStr.insert(0, std::to_string(timeStruct->tm_min));
    if(timeStruct->tm_min < 10)
        formattedLogStr.insert(0, ":0");
    else 
        formattedLogStr.insert(0, 1, ':');
    // add hour
    formattedLogStr.insert(0, std::to_string(timeStruct->tm_hour));
    if(timeStruct->tm_hour < 10)
        formattedLogStr.insert(0, 1, '0');
    
    // convert string to wide string
    auto src = formattedLogStr.c_str();
    auto state = std::mbstate_t();
    auto size = std::mbsrtowcs(nullptr, &src, 0, &state) + 1;

    std::vector<wchar_t> dest(size);
    const auto wide_length = std::mbsrtowcs(dest.data(), &src, size, &state);

    // print to log view
    _logViewPtr->log(std::wstring{dest.data(), wide_length}, static_cast<FLogView::LogLevel>(level));
}


}
