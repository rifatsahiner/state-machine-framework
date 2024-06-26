
#include <cassert>

#include "uOS.h"
#include "logger.h"


namespace uOS {


Task::Task(const TaskId taskId, std::string&& taskName) : _id{taskId}, _name{std::move(taskName)} {
    Logger::addTaskLogger(_id, _name);
}

Task::~Task() {
    Logger::removeTaskLogger(_id);
}

void Task::start(void)
{
    assert(_isRunning == false);

    // clear event queue
    std::queue<std::shared_ptr<const Event>> empty;
    std::swap(_eventQueue, empty);

    init();

    resume();
}

void Task::putEvent(std::shared_ptr<const Event>&& newEvent)
{    
    // lock the queue
    std::lock_guard<std::mutex> lock(_queueMutex);

    // put event into te queue
    //std::cout << "putEvent before - event copy count = " << newEvent.use_count() << std::endl;
    _eventQueue.emplace(std::move(newEvent));
    //std::cout << "putEvent after event copy count = " << _eventQueue.back().use_count() << std::endl;
    
    if(_isUnlocked == false){
        // std::cout << "notify..." << std::endl; 
        std::lock_guard<std::mutex> threadLock (_threadMutex);
        _isUnlocked = true;
        _taskCv.notify_one();
    }
}

void Task::stop(void) 
{    
    // bool isEmpty;

    // lock the queue and store empty info for notify
    // std::lock_guard<std::mutex> lock(_queueMutex);
    // isEmpty = _eventQueue.empty();
    
    _isRunning = false;
    
    if(_isUnlocked == false){
        std::lock_guard<std::mutex> threadLock (_threadMutex);
        _isUnlocked = true;
        _taskCv.notify_one();
    }
    _taskThead.join();  // todo: bool bir parametre ile bekleme olmadan da çıkılabilecek
}

void Task::resume(void) {
    if(_isRunning == false)
    {
        _isRunning = true;
        _taskThead = std::thread {&Task::_taskLoop, this};
        // hem isRunning hem de _getEvent==nullptr condition_variable ile nasıl handle ediyor qp'ye bak
    }
}

void Task::subscribe(SignalId signalId) { FW::subscribe(signalId, _id); }

void Task::unsubscribe(SignalId signalId) { FW::unsubscribe(signalId, _id); }

// todo: bunu _putEvent yapabilir miyiz?
// bu değişecek const Event* alacak, içeride FW::post çağıracak
// yada postSelf olabilir
void Task::putEvent(std::shared_ptr<const Event>& event) { putEvent(std::move(event)); }

// void Task::log__(const char* file, const char* function, int line, std::string& logStr, LogLevel level) {
//     logStr.insert(0, _logPrefix);
//     FW::log__(file, function, line, logStr, level);
// }

//////////////////////////////////////////////////////
//                                                  //
//////////////////////////////////////////////////////

void Task::_taskLoop(void)
{
    // create thread lock
    std::unique_lock<std::mutex> threadLock(_threadMutex);
    _isUnlocked = true;

    //std::cout << "task thread loop started for task: " << _name << std::endl;
    LOG(LogLevel::LOG_INFO, "task thread loop started !!!");

    while(_isRunning)
    {
        // get and handle event. suspend if queue is empty
        std::shared_ptr<const Event> currentEvent = _getEvent();
        
        if(currentEvent != nullptr) {
            //std::cout << "_taskLoop - event copy count = " << currentEvent.use_count() << std::endl;
            dispatch(currentEvent);
        } else {
            //std::cout << "taskId: " << unsigned(_taskId) << " queue is empty, will sleep" << std::endl;
            LOG(LogLevel::LOG_TRACE, "queue is empty, will sleep");
            currentEvent.reset();
            _isUnlocked = false;
            _taskCv.wait(threadLock, [this] {return _isUnlocked;});
            //std::cout << "taskId: " << _taskId << " awake" << std::endl;   
            LOG(LogLevel::LOG_TRACE, "awake");
        }
    }
    
    threadLock.unlock();
    //std::cout << "task thread loop ended for taskId:" << unsigned(_taskId) << std::endl;
    LOG(LogLevel::LOG_INFO, "task thread loop ended !!!");
    return;
}

std::shared_ptr<const Event> Task::_getEvent(void)
{
    std::shared_ptr<const Event> returnEvent;

    // lock the queue
    std::lock_guard<std::mutex> lock(_queueMutex);
    
    if(_eventQueue.empty() == false)
    {
        returnEvent = std::move(_eventQueue.front());
        //std::cout << "get event - event copy count = " << returnEvent.use_count() << std::endl;
        _eventQueue.pop();
        
        //std::cout << "get event on taskId: " << unsigned(_taskId) << " --> signalId: " << returnEvent->signal << std::endl;
        LOG(LogLevel::LOG_TRACE, "get event -> signalId: {}", returnEvent->signal);
    }

    return returnEvent;  // move ediliyor (NRVO)
}


}   // namespace uOS