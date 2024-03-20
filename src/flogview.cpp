
#include "flogview.h"

//////////////////////////////////////////////////////////////////////
//                                                                  //
//////////////////////////////////////////////////////////////////////

FLogView::FLogView(finalcut::FWidget* parent, uint_fast16_t logSize) : finalcut::FDialog{parent}, _logSize{logSize}
{
    //
    // -widget config-
    //
    _textView.addCallback("mouse-wheel-up", this, &FLogView::_loggerScrollUpCb);
    //_textView.ignorePadding();

    _toggleAutoScroll.setChecked(true);
    _toggleAutoScroll.addCallback("toggled", this, &FLogView::_autoScrollToggleCb);

    _buttonPlay.addCallback("clicked", this, &FLogView::_playButtonCb);
    _buttonPlay.setForegroundColor(finalcut::FColor::White);
    _buttonPlay.setFocusForegroundColor(finalcut::FColor::White);
    _buttonPlay.setFocus();

    _buttonClear.addCallback("clicked", this, &FLogView::clear);
    _buttonClear.setForegroundColor(finalcut::FColor::White);
    _buttonClear.setFocusForegroundColor(finalcut::FColor::White);

    _info.setChecked();
    _trace.addCallback("clicked", this, &FLogView::_logLevelClickCb, LogLevel::LOG_TRACE);
    _info.addCallback("clicked", this, &FLogView::_logLevelClickCb, LogLevel::LOG_INFO);
    _warning.addCallback("clicked", this, &FLogView::_logLevelClickCb, LogLevel::LOG_WARNING);
    _error.addCallback("clicked", this, &FLogView::_logLevelClickCb, LogLevel::LOG_ERROR);

    _lineEditFilter.setLabelText(L"Filter");
    _lineEditFilter.setLabelOrientation(finalcut::FLineEdit::LabelOrientation::Above);
    _lineEditFilter.unsetShadow();
    _lineEditFilter.addCallback("changed", this, &FLogView::_filterChangedCb);

    //
    // -static layout-
    //
    // play/pause button
    _buttonPlay.setGeometry(finalcut::FPoint{3,2}, finalcut::FSize{8, 1});

    // play/pause indicator
    _labelPlay.setGeometry(finalcut::FPoint{13,2}, finalcut::FSize{3, 1});

    // clear button
    _buttonClear.setGeometry(finalcut::FPoint{17,2}, finalcut::FSize{7, 1});

    // button group
    _radiobutton_group.setGeometry(finalcut::FPoint{27,1}, finalcut::FSize{29, 3});
    _error.setGeometry(finalcut::FPoint{1,1}, finalcut::FSize{6, 1});
    _warning.setGeometry(finalcut::FPoint{8,1}, finalcut::FSize{6, 1});
    _info.setGeometry(finalcut::FPoint{15,1}, finalcut::FSize{6, 1});
    _trace.setGeometry(finalcut::FPoint{22,1}, finalcut::FSize{6, 1});

    // filter box
    _lineEditFilter.setGeometry(finalcut::FPoint{58,2}, finalcut::FSize{16, 3});
}

//////////////////////////////////////////////////////////////////////
//                                                                  //
//////////////////////////////////////////////////////////////////////

void FLogView::log(std::wstring&& logLine, LogLevel logLevel) {
    bool isShifted {false};
    bool isPrinted {false};
    
    if(_isPlaying == false){
        return;
    }

    if(static_cast<uint_fast8_t>(logLevel) < static_cast<uint_fast8_t>(_currentLogLevel)){
        return;
    }

    // lock logger view access
    std::lock_guard<std::mutex> lg(_loggerViewMtx);

    // rollover if log display is reached
    if(_currentLogSize < _logSize){
        _currentLogSize++;
    } else {
        //remove oldest log from display
        if(_searchString.empty()){
            // if search is not active than remove oldest log
            _textView.deleteLine(0);
            isShifted = true;
        } else {
            // if search is active than remove oldest log only if it matches the search string
            const std::wstring& oldestLog = _mainLogList.front().logString;
            if(oldestLog.find(_searchString, oldestLog.find_first_of(L'|', 16) + 3) != std::string::npos){
                _textView.deleteLine(0);
                isShifted = true;
            }
        }

        //remove oldest log from main log list
        _mainLogList.pop_front();
    }

    // add log to display if necessary
    if(_searchString.empty()){
        _printLog(logLine, logLevel);
        isPrinted = true;
    } else {
        std::string::size_type pos = logLine.find(_searchString, logLine.find_first_of(L'|', 16) + 3);
        if(pos != std::string::npos){
            _printLog(logLine, logLevel, pos);
            isPrinted = true;
        } 
    }

    if(isPrinted){
        if(_autoScroll){
            _textView.scrollToEnd();
        } else {
            // scroll up by 1 to keep the same view when a new log is added and oldest one removed
            if(isShifted && (_textView.getScrollPos().getY() > 0)) {
                _textView.scrollBy(0, -1);
            }
        }
        _textView.redraw();
    } else {
        if(isShifted){
            _textView.redraw();
        }
    }

    // add log to main list
    _mainLogList.emplace_back(LogItem{logLevel, std::move(logLine)});
}

void FLogView::clear(void) {
    // lock logger view access
    std::lock_guard<std::mutex> lg(_loggerViewMtx);
    _textView.clear();
    _mainLogList.clear();
    _currentLogSize = 0;
}

//////////////////////////////////////////////////////////////////////
//                                                                  //
//////////////////////////////////////////////////////////////////////

void FLogView::initLayout(void) {
    _adjust();
    finalcut::FDialog::initLayout();
}

void FLogView::adjustSize(void) {
    _adjust();
    finalcut::FDialog::adjustSize();
}

//////////////////////////////////////////////////////////////////////
//                                                                  //
//////////////////////////////////////////////////////////////////////

void FLogView::_adjust(void) {
    // auto scroll toggle
    finalcut::FPoint togglePosition{static_cast<int>(getClientWidth()) - 15, 1};
    finalcut::FSize toggleSize{15, 3};
    _toggleGroup.setGeometry(togglePosition, toggleSize);
    _toggleAutoScroll.setGeometry(finalcut::FPoint{2,1}, finalcut::FSize{10, 1});

    // logger view
    finalcut::FPoint loggerPosition{1,4};
    finalcut::FSize loggerSize{getWidth(), getClientHeight() - 3};
    _textView.setGeometry(loggerPosition, loggerSize);
}

void FLogView::_printLog(const std::wstring& logLine, LogLevel logLevel, std::string::size_type hglPos){
    finalcut::FString logLineFstring{logLine};

    switch (logLevel) {
    case LogLevel::LOG_TRACE:
        logLineFstring.insert(L"[TRACE]   ", 0);
        _textView.append(logLineFstring);    
        _textView.addHighlight(_textView.getLines().size()-1, finalcut::FTextView::FTextHighlight{0, 7, finalcut::FColorPair{finalcut::FColor::White, finalcut::FColor::LightGray}});
        break;

    case LogLevel::LOG_INFO:
        logLineFstring.insert(L"[INFO]    ", 0);
        _textView.append(logLineFstring);    
        _textView.addHighlight(_textView.getLines().size()-1, finalcut::FTextView::FTextHighlight{0, 6, finalcut::FColorPair{finalcut::FColor::White, finalcut::FColor::Blue}});
        break;

    case LogLevel::LOG_WARNING:
        logLineFstring.insert(L"[WARNING] ", 0);
        _textView.append(logLineFstring);    
        _textView.addHighlight(_textView.getLines().size()-1, finalcut::FTextView::FTextHighlight{0, 9, finalcut::FColorPair{finalcut::FColor::White, finalcut::FColor::DarkOrange}});
        break;

    case LogLevel::LOG_ERROR:
        logLineFstring.insert(L"[ERROR]   ", 0);
        _textView.append(logLineFstring);    
        _textView.addHighlight(_textView.getLines().size()-1, finalcut::FTextView::FTextHighlight{0, 7, finalcut::FColorPair{finalcut::FColor::White, finalcut::FColor::Red}});
        break;

    default:
        _textView.append(L"[UNDEF]    PRINT LOG ATTEMPT WITH UNKNOWN LOG TYPE");    
        _textView.addHighlight(_textView.getLines().size()-1, finalcut::FTextView::FTextHighlight{0, 7, finalcut::FColorPair{finalcut::FColor::White, finalcut::FColor::Purple}});
        return;
    }

    if(hglPos != std::string::npos){
        _textView.addHighlight(_textView.getLines().size()-1, finalcut::FTextView::FTextHighlight{hglPos + 10, _searchString.length(), finalcut::FColorPair{finalcut::FColor::Black, finalcut::FColor::Yellow}});
    }
}

//////////////////////////////////////////////////////////////////////
//                                                                  //
//////////////////////////////////////////////////////////////////////

void FLogView::_autoScrollToggleCb(void) {
    _autoScroll = _toggleAutoScroll.isChecked();
}

void FLogView::_loggerScrollUpCb(void) {
    if(_autoScroll){
        _autoScroll = false;
        _toggleAutoScroll.setChecked(false);
        _toggleAutoScroll.redraw();
    }
}

void FLogView::_playButtonCb(void){
    if(_isPlaying){
        _isPlaying = false;
        _labelPlay.setText(finalcut::FString{std::wstring{L"\U0001F534"}});
    } else {
        _isPlaying = true;
        _labelPlay.setText(finalcut::FString{std::wstring{L"\U0001F7E2"}});
    }
    _labelPlay.redraw();
}

void FLogView::_logLevelClickCb(LogLevel newLogLevel) {
    if(newLogLevel != _currentLogLevel) {
        if(static_cast<uint_fast8_t>(newLogLevel) > static_cast<uint_fast8_t>(_currentLogLevel)) {
            bool emptyFlag = _searchString.empty();
            
            // lock logger view access
            std::lock_guard<std::mutex> lg(_loggerViewMtx);
            
            // clear logger view
            _textView.clear();

            // remove lower level logs and print others
            for(auto it = _mainLogList.cbegin(); it != _mainLogList.cend();) {
                if(static_cast<uint_fast8_t>(it->logLevel) < static_cast<uint_fast8_t>(newLogLevel)) {
                    it = _mainLogList.erase(it);
                } else {
                    if(emptyFlag) {
                        _printLog(it->logString, it->logLevel);
                    } else {
                        std::string::size_type pos = it->logString.find(_searchString, it->logString.find_first_of(L'|', 16) + 3);
                        if(pos != std::string::npos){
                            _printLog(it->logString, it->logLevel, pos);
                        }
                    }
                    it++;
                }
            }

            // scroll if needed and redraw logger view
            if(_autoScroll){
                _textView.scrollToEnd();
            }
            _textView.redraw();
        }

        _currentLogLevel = newLogLevel;
    }
}

void FLogView::_filterChangedCb(void){
    bool clearFlag{false};

    // lock logger view access
    std::lock_guard<std::mutex> lg(_loggerViewMtx);
    
    _searchString = _lineEditFilter.getText().toWString();
    if(_searchString == L" "){
        _searchString.clear();
        clearFlag = true;
    }
    
    // clear logger
    _textView.clear();

    // search main list and print matching logs
    for(auto it = _mainLogList.cbegin(); it != _mainLogList.cend(); it++){
        if(clearFlag){
            _printLog(it->logString, it->logLevel);
        } else {
            // start search after log string seperator
            std::string::size_type pos = it->logString.find(_searchString, it->logString.find_first_of(L'|', 16) + 3);  // 16 -> minimum pos that seperator can be
            if(pos != std::string::npos){
                _printLog(it->logString, it->logLevel, pos);
            }
        }
    }

    // scroll if needed and redraw logger view
    if(_autoScroll){
        _textView.scrollToEnd();
    }
    _textView.redraw();
}
