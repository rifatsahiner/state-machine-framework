
#include <vector>
#include <cassert>
#include <algorithm>

#include "state_machine.h"
#include "logger.h"


namespace uOS {


static constexpr SignalId EMPTY_SIG = static_cast<SignalId>(0);
static constexpr Event EMPTY_EVENT {EMPTY_SIG, std::nullopt};
static const auto EMPTY_EVENT_PTR = std::make_shared<const Event>(EMPTY_EVENT);

static constexpr Event ENTRY_EVENT {static_cast<SignalId>(1), std::nullopt};
static const auto ENTRY_EVENT_PTR = std::make_shared<const Event>(ENTRY_EVENT);

static constexpr Event EXIT_EVENT {static_cast<SignalId>(2), std::nullopt};
static const auto EXIT_EVENT_PTR = std::make_shared<const Event>(EXIT_EVENT);

static constexpr Event INIT_EVENT {static_cast<SignalId>(3), std::nullopt};
static const auto INIT_EVENT_PTR = std::make_shared<const Event>(INIT_EVENT);


void StateMachine::init(void)
{
    HandleResult res;

    // check if current state is abs-top
    assert(_currentState == &StateMachine::top);

    // call initial top transition 
    // assert if transition is not provided
    res = _topInitialTrans();
    assert(res == HandleResult::Trans);

    // process initial transition
    _currentState = _procInitTrans();
}

void StateMachine::dispatch(const std::shared_ptr<const Event>& event)
{
    State sourceState;
    State targetState;
    HandleResult res;

    // check state integrity
    assert(_currentState == _tempTargetState);

    // pass event up throgh the hierarchy until handled or ignored by top
    do {
        sourceState = _tempTargetState;
        res = (this->*sourceState)(event);

        if(res == HandleResult::Unhandled) {
            res = (this->*sourceState)(EMPTY_EVENT_PTR);
        }
    } while (res == HandleResult::Super);

    // check if event resulted in a transition
    if(res == HandleResult::Trans)
    {
        std::vector<State> entryPath;
        State tempStateHolder = _currentState;
        
        // save target state
        targetState = _tempTargetState;

        // exit states until reaching transition source state
        while(tempStateHolder != sourceState)
        {
            if((this->*tempStateHolder)(EXIT_EVENT_PTR) == HandleResult::Handled){
                // if exit event is handled than put empty event to get next superstate
                (this->*tempStateHolder)(EMPTY_EVENT_PTR);
            }
            tempStateHolder = _tempTargetState;
        }

        //  --- (a) source is the target (a) ---
        // check if source is the targer
        if(sourceState == targetState){
            // exit source and enter the target
            (this->*sourceState)(EXIT_EVENT_PTR);
            entryPath.push_back(targetState);
            //std::cout << "\nTransition A happened\n" << std::endl;
            LOG(LogLevel::LOG_TRACE, "[SYSTEM] Transition A");
        } else 
        {
            // --- (b) source is the superstate of target (b) ---
            // get target's superstate and check if it is the source
            (this->*targetState)(EMPTY_EVENT_PTR);
            if(sourceState == _tempTargetState){
                // just enter the target
                entryPath.push_back(targetState);
                //std::cout << "\nTransition B happened\n" << std::endl;
                LOG(LogLevel::LOG_TRACE, "[SYSTEM] Transition B");
            } else 
            {
                // --- (c) source and target are at the same level (c) ---
                // check if superstates of source and target are the same
                tempStateHolder = _tempTargetState;     // save superstate of target (from clause b) in tempStateHolder
                (this->*sourceState)(EMPTY_EVENT_PTR);
                if(tempStateHolder == _tempTargetState){
                    // exit source and enter the target
                    (this->*sourceState)(EXIT_EVENT_PTR);
                    entryPath.push_back(targetState);
                    //std::cout << "\nTransition C happened\n" << std::endl;
                    LOG(LogLevel::LOG_TRACE, "[SYSTEM] Transition C");
                } else 
                {
                    // --- (d) target is the superstate of source (d) ---
                    // check if source's superstate (from clause c) is the same with target 
                    if(_tempTargetState == targetState){
                        // just exit the source
                        (this->*sourceState)(EXIT_EVENT_PTR);
                        //std::cout << "\nTransition D happened\n" << std::endl;
                        LOG(LogLevel::LOG_TRACE, "[SYSTEM] Transition D");
                    } else {
                        bool isLcaFound = false;
                        
                        // --- (e) source is multiple level higher than target (e) ---
                        entryPath = {targetState, tempStateHolder};     // put target state end its super (from clause c) to path
                        tempStateHolder = _tempTargetState;             // save source's superstate in tempStateHolder

                        // go higher from target until finding source or reaching the abs-top (f or g in that case)
                        res = (this->*entryPath[1])(EMPTY_EVENT_PTR);
                        while(res == HandleResult::Super)
                        {
                            // is this the source?
                            if(_tempTargetState == sourceState)
                            {
                                // LCA is the source (means case is "e")
                                isLcaFound = true;
                                //std::cout << "\nTransition E happened\n" << std::endl;
                                LOG(LogLevel::LOG_TRACE, "[SYSTEM] Transition E");
                                break;
                            } else
                            {
                                // add state to path and keep going up
                                entryPath.push_back(_tempTargetState);
                                res = (this->*_tempTargetState)(EMPTY_EVENT_PTR);
                            }
                        }

                        // if LCA is not found check if its case (f) or (g)
                        if(isLcaFound == false)
                        {
                            // exit source
                            (this->*sourceState)(EXIT_EVENT_PTR);
                            
                            // check if source's super (stored in tempStateHolder in clause e) is on the path
                            auto lcaIter = std::find(entryPath.cbegin() + 2, entryPath.cend(), tempStateHolder);     // skip target and target's super
                            if(lcaIter != entryPath.cend())
                            {
                                // --- (f) LCA is source's super (f) ---
                                // remove source's super and higher states from path
                                entryPath.erase(lcaIter, entryPath.end());
                                //std::cout << "\nTransition F happened\n" << std::endl;
                                LOG(LogLevel::LOG_TRACE, "[SYSTEM] Transition F");
                            } else
                            {
                                // --- (g)(h) LCA is multiple levels higher than source (g)(h) ---
                                // iterate through source's superstates until finding one in the path
                                do {
                                    // exit source's current superstate an climb one superstate higher
                                    if((this->*tempStateHolder)(EXIT_EVENT_PTR) == HandleResult::Handled) {
                                        (this->*tempStateHolder)(EMPTY_EVENT_PTR);
                                    }
                                    tempStateHolder = _tempTargetState;

                                    // check if this superstate is on the path
                                    lcaIter = std::find(entryPath.cbegin(), entryPath.cend(), tempStateHolder);     // lca can be target (clause h), so no skipping like above
                                } while(lcaIter == entryPath.cend());

                                // remove LCA and higher states from path
                                entryPath.erase(lcaIter, entryPath.end());
                                //std::cout << "\nTransition G/H happened\n" << std::endl;
                                LOG(LogLevel::LOG_TRACE, "[SYSTEM] Transition G/H");
                            }
                        }
                    }
                }
            }
        }

        // enter path top->down than set target as the current state
        std::for_each(entryPath.crbegin(), entryPath.crend(), [&](const State& state){ (this->*state)(ENTRY_EVENT_PTR); });
        _currentState = targetState;

        // process initial transition if target is a composite state
        if((this->*targetState)(INIT_EVENT_PTR) == HandleResult::Trans){
            _currentState = _procInitTrans();
        } else {
            _tempTargetState = _currentState;
        }
    }

    // restore temp state if no trasition taken
    _tempTargetState = _currentState;
}

bool StateMachine::isIn(const State state)
{
    // check state integrity
    assert(_currentState == _tempTargetState);

    bool isIn = false;

    do {
        if(_tempTargetState == state) {
            // state is matched, set result and break loop
            isIn = true;
            break;
        }
        
        // loop until checking top state 
    } while ((this->*_tempTargetState)(EMPTY_EVENT_PTR) != HandleResult::Ignored);

    // restore temp state and return result
    _tempTargetState = _currentState;
    return isIn;
}

//
// processes initial transition for current composite state
// returns resulted leaf state
// (should be called after handling initial transition event)
//
State StateMachine::_procInitTrans(void)
{
    State currentTopState = _currentState;

    do {
        std::vector<State> path {_tempTargetState};

        // get target's super
        (this->*_tempTargetState)(EMPTY_EVENT_PTR);

        // go up until reaching current top
        while(_tempTargetState != currentTopState) {
            // save new state to path and get super of that state
            path.push_back(_tempTargetState);
            (this->*_tempTargetState)(EMPTY_EVENT_PTR);
        }

        // restore target state
        _tempTargetState = path[0];

        // enter states in the path top down
        std::for_each(path.crbegin(), path.crend(), [&](const State& state){ (this->*state)(ENTRY_EVENT_PTR); });

        // set new top state as the target
        currentTopState = path[0];

        // continue to iterate until reaching a leaf state that ignores init signal
    } while ((this->*currentTopState)(INIT_EVENT_PTR) == HandleResult::Trans);

    // return final leaf state
    _tempTargetState = currentTopState;
    return currentTopState;
}


}   // namespace uOS
