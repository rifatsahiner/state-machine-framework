
#include "ulak.h"
#include <iostream>

namespace uOS {


Task::Task(const TaskId taskId, const std::string& taskName) : _taskId{taskId}, _name{std::move(taskName)} { }

void Task::start(void)
{
    assert(_isRunning == false);

    // clear event queue
    std::queue<std::shared_ptr<const Event>> empty;
    std::swap(_eventQueue, empty);

    _init();

    resume();
}

void Task::putEvent(std::shared_ptr<const Event>&& newEvent)
{    
    // lock the queue
    std::lock_guard<std::mutex> lock(_queueMutex);

    // put event into te queue
    std::cout << "putEvent before - event copy count = " << newEvent.use_count() << std::endl;
    _eventQueue.emplace(std::move(newEvent));
    std::cout << "putEvent after event copy count = " << _eventQueue.back().use_count() << std::endl;
    
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
}

void Task::resume(void) {
    if(_isRunning == false)
    {
        _isRunning = true;
        _taskThead = std::thread {&Task::_taskLoop, this};
        // hem isRunning hem de _getEvent==nullptr condition_variable ile nasıl handle ediyor qp'ye bak
    }
}

void Task::subscribe(SignalId signalId) { FW::subscribe(signalId, _taskId); }

void Task::unsubscribe(SignalId signalId) { FW::unsubscribe(signalId, _taskId); }

// todo: bunu _putEvent yapabilir miyiz?
void Task::putEvent(std::shared_ptr<const Event>& event) { putEvent(std::move(event)); }

//////////////////////////////////////////////////////
//                                                  //
//////////////////////////////////////////////////////

void Task::_taskLoop(void)
{
    // create thread lock
    std::unique_lock<std::mutex> threadLock(_threadMutex);
    _isUnlocked = true;

    std::cout << "task thread loop started for task: " << _name << std::endl;

    while(_isRunning)
    {
        // get and handle event. suspend if queue is empty
        std::shared_ptr<const Event> currentEvent = _getEvent();
        
        if(currentEvent != nullptr) {
            std::cout << "_taskLoop - event copy count = " << currentEvent.use_count() << std::endl;
            _stateMachine(currentEvent);
        } else {
            std::cout << "taskId: " << unsigned(_taskId) << " queue is empty, will sleep" << std::endl;
            currentEvent.reset();
            _isUnlocked = false;
            _taskCv.wait(threadLock, [this] {return _isUnlocked;});
            std::cout << "taskId: " << _taskId << " awake" << std::endl;   
        }
    }
    
    threadLock.unlock();
    std::cout << "task thread loop ended for taskId:" << unsigned(_taskId) << std::endl;
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
        std::cout << "get event - event copy count = " << returnEvent.use_count() << std::endl;
        _eventQueue.pop();
        
        std::cout << "get event on taskId: " << unsigned(_taskId) << " --> signalId: " << returnEvent->signal << std::endl;
    }

    return returnEvent;  // move ediliyor (NRVO)
}


}   // namespace uOS