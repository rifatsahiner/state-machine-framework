
#include "logger.h"
#include "flogview.h"

void tuiThreadFunc(void) {
    finalcut::FApplication::getApplicationObject()->exec();
}

namespace uOS {


void Logger::init(int argc, char* argv[])
{
    static finalcut::FApplication app(argc, argv);

    // Force terminal initialization without calling show()
    app.initTerminal();

    // create logger
    static FLogView logger(&app, 300);
    finalcut::FWidget::setMainWidget(&logger);
    _loggerHandle = &logger;

    // configure loger
    logger.setText(L"Logger");
    logger.unsetShadow();
    logger.setResizeable(true);
    logger.setGeometry(finalcut::FPoint{1,1}, finalcut::FSize{app.getDesktopWidth(), app.getDesktopHeight()});
    logger.show();
}

void Logger::start(void) {
    _loggerThread = std::thread{tuiThreadFunc};
}

void Logger::stop(void) {
    finalcut::FApplication::getApplicationObject()->quit();
    _loggerThread.join();
}

void Logger::log(const char* file, const char* function, int line, LogLevel level, std::string& formattedLogStr) {
    // Get the current time and convert to time struct
    auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::tm* timeStruct = std::localtime(&now);

    // "HH:MM:SS FILE:LINE:FUNC logstr"
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
    _loggerHandle->log(std::wstring{dest.data(), wide_length}, static_cast<FLogView::LogLevel>(level));
}


}
