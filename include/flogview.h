
#ifndef F_LOG_VIEW_H
#define F_LOG_VIEW_H

#include "final/final.h"

class FLogView : public finalcut::FDialog
{
  public:
    explicit FLogView(finalcut::FWidget* = nullptr, uint_fast16_t scrollBackLimit = DEFAULT_LOG_BUFFER_SIZE);

    enum class LogLevel : uint_fast8_t {
      LOG_TRACE = 0,
      LOG_INFO,
      LOG_WARNING,
      LOG_ERROR
    };

    void log(std::wstring&& logLine, LogLevel logLevel);
    void clear(void);

  protected:
    void initLayout(void) override;
    void adjustSize(void) override;

  private:
    void _printLog(const std::wstring&, LogLevel, std::string::size_type = std::string::npos);
    void _adjust(void);

    void _autoScrollToggleCb(void);
    void _loggerScrollUpCb(void);
    void _playButtonCb(void);
    void _logLevelClickCb(LogLevel);
    void _filterChangedCb(void);

    // widgets
    finalcut::FTextView _textView{this};
    finalcut::FButton _buttonPlay{L"▶/▮▮", this};
    finalcut::FLabel _labelPlay{L"🟢", this};
    finalcut::FButton _buttonClear{L"Clear", this};
    finalcut::FButtonGroup _radiobutton_group {L"Log Level", this};
    finalcut::FRadioButton _error {"E", &_radiobutton_group};
    finalcut::FRadioButton _warning {"W", &_radiobutton_group};
    finalcut::FRadioButton _info {"I", &_radiobutton_group};
    finalcut::FRadioButton _trace {"T", &_radiobutton_group};
    finalcut::FLineEdit _lineEditFilter {this};
    finalcut::FButtonGroup _toggleGroup {L"Auto Scroll", this};
    finalcut::FSwitch _toggleAutoScroll{&_toggleGroup};

    // data members
    const uint_fast16_t _logSize;
    uint_fast16_t _currentLogSize{0};
    bool _autoScroll{true};
    bool _isPlaying{true};
    std::wstring _searchString;
    LogLevel _currentLogLevel{LogLevel::LOG_INFO};
    std::mutex _loggerViewMtx;

    struct LogItem {
      LogLevel logLevel;
      std::wstring logString;
    };
    std::list<LogItem> _mainLogList;

    static constexpr uint_fast16_t DEFAULT_LOG_BUFFER_SIZE = 2000;
};

// append with FString r-value referance
// moving a whole FTextViewList - this will call clear and than use new text list
// FSwitch text orientation olmalı

#endif  // F_LOG_VIEW_H