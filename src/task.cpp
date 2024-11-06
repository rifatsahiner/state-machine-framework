
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
    _eventQueue.emplace(std::move(newEvent));
    
    if(_isUnlocked == false){
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

//////////////////////////////////////////////////////
//                                                  //
//////////////////////////////////////////////////////

void Task::_taskLoop(void)
{
    // create thread lock
    std::unique_lock<std::mutex> threadLock(_threadMutex);
    _isUnlocked = true;

    LOG(LogLevel::LOG_INFO, "task thread loop started !!!");

    while(_isRunning)
    {
        // get and handle event. suspend if queue is empty
        std::shared_ptr<const Event> currentEvent = _getEvent();
        
        if(currentEvent != nullptr) {
            dispatch(currentEvent);
        } else {
            LOG(LogLevel::LOG_TRACE, "queue is empty, will sleep");
            currentEvent.reset();
            _isUnlocked = false;
            _taskCv.wait(threadLock, [this] {return _isUnlocked;});
            LOG(LogLevel::LOG_TRACE, "awake");
        }
    }
    
    threadLock.unlock();
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
        
        LOG(LogLevel::LOG_TRACE, "get event -> signalId: {}", returnEvent->signal);
    }

    return returnEvent;  // move ediliyor (NRVO)
}


}   // namespace uOS