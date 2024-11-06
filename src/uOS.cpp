
#include <iterator>
#include <algorithm>
#include <chrono>

#include "uOS.h"

// #define ASSERT_ON_EXTRA_COPY(event_ptr, target_use_count) assert(event_ptr.use_count() == target_use_count)
using namespace std::chrono_literals;


namespace uOS {


void FW::init(int argc, char* argv[]){
    // init usage lists
    _taskIdUsageList.fill(false);
    _timerIdUsageList.fill(false);

    // init logger if activated
    if(argc != 0){
        Logger::init(argc, argv, &FW::kill);
    }

    // lock memory so we're never swapped out to disk   ????
    //mlockall(MCL_CURRENT | MCL_FUTURE);          // uncomment when supported
}

int32_t FW::run() {
    const Event TICK_SEC_EVENT {SIG_TICK_SEC, std::nullopt};
    const Event TICK_MIN_EVENT {SIG_TICK_MIN, std::nullopt};
    const Event TICK_HOUR_EVENT {SIG_TICK_HOUR, std::nullopt};
    uint_fast8_t s = 0;
    uint_fast8_t m = 0;
    uint_fast8_t h = 0;

    // onStartup() aşağıdaki satırlar
    // struct termios tio;                        // modified terminal attributes
    // tcgetattr(0, &l_tsav);             // save the current terminal attributes
    // tcgetattr(0, &tio);              // obtain the current terminal attributes
    // tio.c_lflag &= ~(ICANON | ECHO);      // disable the canonical mode & echo
    // tcsetattr(0, TCSANOW, &tio);                     // set the new attributes
    // QF_setTickRate(DPP::BSP_TICKS_PER_SEC);       // set the desired tick rate

    // set thread priority ??? aşağıdaki satırlar
    // struct sched_param sparam;
    // sparam.sched_priority = sched_get_priority_max(SCHED_FIFO);
    // pthread_setschedparam(pthread_self(), SCHED_FIFO, &sparam) == 0;  

    // start logger
    Logger::start();
    LOG(uOS::LogLevel::LOG_INFO, "[SYSTEM] Welcome to uOS application...");

    while(_isRunning) {
        // now süresini al
        auto now =  std::chrono::high_resolution_clock::now();

        // check for second
        if(++s == 50) {
            auto secEvent = std::make_shared<const Event>(TICK_SEC_EVENT);
            _publishEvent(secEvent);
            s = 0;
            
            // check for minute
            if(++m == 60) {
                auto minEvent = std::make_shared<const Event>(TICK_MIN_EVENT);
                _publishEvent(minEvent);
                m = 0;

                // check for hour
                if(++h == 60) {
                    auto hourEvent = std::make_shared<const Event>(TICK_HOUR_EVENT);
                    _publishEvent(hourEvent);
                    h = 0;
                }
            }
        }

        // handle timers
        _handleTick();

        // sleep_until now + 20ms
        std::this_thread::sleep_until(now + 20ms);
    }

    // terminate all task threads
    // todo: buraya tüm task threadleri kapatacak bir kod eklenecek
    //stopTask(_taskMap.begin()->first);
    for(auto it = _taskMap.begin(); it != _taskMap.end();) {
        stopTask(it->first);
        it = _taskMap.erase(it);
    }

    // terminate logger
    LOG(LogLevel::LOG_INFO, "[SYSTEM] uOS Bye bye!");
    Logger::stop();

    // onCleanup aşağıdaki satırlar
    // tcsetattr(0, TCSANOW, &l_tsav);   // restore the saved terminal attributes

    // hangi mutex bu, neden destroy ediliyor ???
    // pthread_mutex_destroy(&QF_pThreadMutex_);

    return 0;
}

void FW::kill(void){
    _isRunning = false;
}

void FW::startTask(TaskId taskId){
    auto taskIter = _taskMap.find(taskId);
    assert(taskIter != _taskMap.cend());

    taskIter->second->start();
}

void FW::stopTask(TaskId taskId){
    auto taskIter = _taskMap.find(taskId);
    assert(taskIter != _taskMap.cend());

    taskIter->second->stop();
}

void FW::postEvent(TaskId taskId, const Event* event){
    auto taskIter = _taskMap.find(taskId);
    assert(taskIter != _taskMap.cend());

    std::shared_ptr<const Event> eventSp(event);
    event = nullptr;

    //taskIter->second->putEvent(std::move(const_cast< std::shared_ptr<Event>& >(event)));
    taskIter->second->putEvent(std::move(eventSp));
}

TimerId FW::postEventIn(uint_fast16_t intervalMs, TaskId taskId, const Event* event)
{
    assert(_taskMap.find(taskId) != _taskMap.cend());
    
    // call common function for new timer creation
    return _setupTimedEvent(false, false, intervalMs / 20, event, taskId);
}

TimerId FW::postEventEvery(uint_fast16_t intervalMs, TaskId taskId, const Event* event)
{
    assert(_taskMap.find(taskId) != _taskMap.cend());

    return _setupTimedEvent(false, true, intervalMs / 20, event, taskId);
}

void FW::publishEvent(const Event* event)
{
    std::lock_guard<std::mutex> lock(_subsMutex);

    // get signal's subs list, assert if signal id does not have subs list 
    auto subListIter = _subsMap.find(event->signal);
    //assert(subListIter != _subsMap.cend()); -> publish eden bunu bilmeyebilir
    // todo: sinyalin subs ları yoksa return edilmesi gerekiyor

    std::shared_ptr<const Event> eventSp(event);
    event = nullptr;

    _publishEventCommon(eventSp, subListIter->second);
}

TimerId FW::publishEventIn(uint_fast16_t intervalMs, const Event* event)
{
    return _setupTimedEvent(true, false, intervalMs / 20, event);
}

TimerId FW::publishEventEvery(uint_fast16_t intervalMs, const Event* event)
{
    return _setupTimedEvent(true, true, intervalMs / 20, event);
}

void FW::cancelTimedEvent(TimerId timerId){
    std::lock_guard<std::mutex> lock(_timerMutex);

    // check if timer id is valid
    if(_timerStorageMap.find(timerId) != _timerStorageMap.cend())
    {
        auto itPrev = _timerQueue.cbefore_begin();

        // find timer in queue
        for (auto itCancel = _timerQueue.begin(); itCancel != _timerQueue.cend(); ++itCancel)
        {
            if(itCancel->first == timerId)
            {
                // add remeaning downcounter value to next timer
                auto itNext = std::next(itCancel);
                if(itNext != _timerQueue.cend())
                    itNext->second += itCancel->second;

                // clear timer data
                _timerQueue.erase_after(itPrev);
                _timerStorageMap.erase(timerId);
                _timerIdUsageList[timerId] = false;

                return;
            }
            else
                itPrev = itCancel;    
        }
        
        assert(false);
    }

    assert(false);
}

void FW::subscribe(SignalId signalId, TaskId subsId)
{
    // assert -- user-signal-start < signalId < signal-list-count
    std::lock_guard<std::mutex> lock(_subsMutex);

    auto signalSubsListIter = _subsMap.find(signalId);

    // check if signal has active subscribers
    if(signalSubsListIter == _subsMap.cend())
    {
        // add signal to the map along with its first subscriber 
        std::vector<TaskId> subsList;
        subsList.push_back(subsId);
        _subsMap.emplace(std::move(signalId), std::move(subsList));
    } else 
    {
        std::vector<TaskId>& subsList = signalSubsListIter->second;

        // assert if subs is already on the list
        assert(std::find(subsList.begin(), subsList.end(), subsId) == subsList.end());

        // add task to the list as a new subs
        subsList.push_back(subsId);
    }

    //std::cout << "Task Id: " << unsigned(subsId) << "subscribed to Signal Id: " << signalId << std::endl;
    LOG(LogLevel::LOG_INFO, "Task Id: {} subscribed to signalId: {}", subsId, signalId);
}

void FW::unsubscribe(SignalId signalId, TaskId subsId)
{
    std::lock_guard<std::mutex> lock(_subsMutex);

    // get signal's subs list, assert if signal id does not have subs list
    auto signalSubsListIter = _subsMap.find(signalId);
    assert(signalSubsListIter != _subsMap.cend());

    // get iterator for subsId, assert if subsId is not present
    std::vector<TaskId>& subsList = signalSubsListIter->second;
    auto subsIdIter = std::find(subsList.cbegin(), subsList.cend(), subsId);
    assert(subsIdIter != subsList.cend());

    // remove subsId from list
    subsList.erase(subsIdIter);
    //std::cout << "taskId " << unsigned(*subsIdIter) << " unsubscibed from signalId " << signalId << std::endl;
    LOG(LogLevel::LOG_INFO, "Task Id: {} unsubscribed to signalId: {}", subsId, signalId);

    //todo: sinyalde subs kalmadıysa map'den sil
}

//////////////////////////////////////////////////////
//                                                  //
//////////////////////////////////////////////////////

void FW::_publishEvent(std::shared_ptr<const Event>& event)
{
    std::lock_guard<std::mutex> lock(_subsMutex);

    // get signal's subs list, return if signal id does not have subs list 
    auto subListIter = _subsMap.find(event->signal);

    if(subListIter != _subsMap.cend()) {
        _publishEventCommon(event, subListIter->second);
    }
}

void FW::_publishEventCommon(std::shared_ptr<const Event>& event, std::vector<TaskId>& subsList)
{
    // for each subs, send a copy of the event (send original event to last subs)
    for(TaskId taskId : subsList){
        if(taskId != subsList.back()){
            std::shared_ptr<const Event> eventCopy = event;
            _taskMap.find(taskId)->second->putEvent(std::move(eventCopy));
        } else 
            _taskMap.find(taskId)->second->putEvent(std::move(event));
    }
}

TaskId FW::_getAvailableTaskId(void){
    TaskId i;
    
    for(i = 0; i < uOS_FW_MAX_TASK_COUNT; i++){
        if(_taskIdUsageList[i] == false){
            _taskIdUsageList[i] = true;
            return i;
        }
    }

    assert(false);
}

TimerId FW::_getAvailableTimerId(void){
    TimerId i;
    
    for(i = 0; i < uOS_FW_MAX_TIMER_COUNT; i++){
        if(_timerIdUsageList[i] == false){
            _timerIdUsageList[i] = true;
            return i;
        }
    }
    
    assert(false);
}

void FW::_insertTimerQueue(TimerId timerId, uint_fast16_t interval)
{
    uint16_t accuPrev {0};
    uint16_t accuNext;

    // lock the queue --> bunu çağıran yerlerde kilitlendi
    // std::lock_guard<std::mutex> lock(_timerMutex);

    auto itPrev = _timerQueue.cbefore_begin();
    auto itNext = _timerQueue.begin();

    for (; itNext != _timerQueue.cend(); ++itNext) {
        accuNext = accuPrev + itNext->second;
        if(interval < accuNext)
            break;
        else {
            accuPrev = accuNext;
            itPrev = itNext;
        }
    }

    // yeni timer'ı interval değerini aşan son iteratörden bir önceye koy
    // sonraki timer'ın delta değerini yeni giren timer'ın delta değeri kadar azalt
    _timerQueue.insert_after(itPrev, std::make_pair(timerId, interval - accuPrev));
    if(itNext != _timerQueue.cend())
        itNext->second = accuNext - interval;
}

TimerId FW::_setupTimedEvent(bool isPublish, bool isPeriodic, uint_fast16_t interval, const Event* eventPtr, TaskId receiverTaskId)
{
    // create shared pointer to use on remeaining life of event
    std::shared_ptr<const Event> eventSp(eventPtr);
    eventPtr = nullptr;

    TimerNode timerNode {isPublish, isPeriodic, receiverTaskId, interval, std::move(eventSp) };
    //ASSERT_ON_EXTRA_COPY(timerNode.eventPtr, 1);

    // lock timer related resources
    std::lock_guard<std::mutex> lock(_timerMutex);

    // acquire timer id
    TimerId timerId = _getAvailableTimerId();

    // insert timer to delta queue
    _insertTimerQueue(timerId, interval);

    // store timer data
    _timerStorageMap.emplace(std::move(timerId), std::move(timerNode));
    //ASSERT_ON_EXTRA_COPY(_timerStorageMap.find(timerId)->second.eventPtr, 1);
    // todo: assert if emplace fails

    //std::cout << "timer created with timer id: " << timerId << std::endl;
    LOG(LogLevel::LOG_TRACE, "Timer created with timerId: {}", timerId);
    return timerId;
}

void FW::_handleTick(void)  // todo: inline yap
{
    std::unique_lock<std::mutex> lock (_timerMutex);    // scooped lock ile hem taskMap mutex hem de timerMutex kilitlenebilir

    if(_timerQueue.empty() == false)
    {
        std::vector<TimerId> expiredTimers;
        TimerNode timerNode;

        // get head timer and decrement downcounter
        auto head = _timerQueue.begin();
        head->second--;

        // check if head expired
        while(head->second == 0)
        {
            // save expired timer id and remove from queue           
            expiredTimers.push_back(head->first);
            _timerQueue.pop_front();

            // get new head or 
            if(_timerQueue.empty() == false)
                head = _timerQueue.begin();
            else
                break;
        }

        // unlock the mutex until there is any timer map/queue access 
        lock.unlock();

        // handle expired timers
        for(TimerId timerId : expiredTimers) 
        {
            //std::cout << "expired timer with timer id: " << unsigned(timerId) << std::endl;  
            LOG(LogLevel::LOG_TRACE, "Timer expired with timerId: {}", timerId); 

            // get stored timer node with the timer key
            TimerNode& timerNode = _timerStorageMap.find(timerId)->second;  // find is thread-safe
            //ASSERT_ON_EXTRA_COPY(timerNode.eventPtr, 1);

            // event'i post et
            // todo: event'i move edersem periyodik olarak tekrar çağırıldığında event kaybolmuyor mu?
            if(timerNode.isPublish){
                if(timerNode.isPeriodic == false)
                    _publishEvent(timerNode.eventPtr);
                else {
                    std::shared_ptr<const Event> eventCopy = timerNode.eventPtr;
                    _publishEvent(eventCopy);
                }
            } else {
                if(timerNode.isPeriodic == false){
                    _taskMap.find(timerNode.receiverTaskId)->second->putEvent(std::move(timerNode.eventPtr));
                }else {
                    std::shared_ptr<const Event> eventCopy = timerNode.eventPtr;
                    _taskMap.find(timerNode.receiverTaskId)->second->putEvent(std::move(eventCopy));
                }
            }

            // periodikse tekrar ekle
            // periyodik değilse storage erase yap
            lock.lock();
            if(timerNode.isPeriodic){
                _insertTimerQueue(timerId, timerNode.interval);
            } else {
                _timerStorageMap.erase(timerId);
                _timerIdUsageList[timerId] = false;
            }
            lock.unlock();
        }
    }
}


}   // namespace uOS
