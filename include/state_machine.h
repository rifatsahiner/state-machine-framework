#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include <cstdint>
#include <memory>


namespace uOS {


using SignalId = uint_fast16_t;

struct Event {
    SignalId signal;
};

enum class HandleResult : uint_fast8_t {
    Handled,
    Unhandled,
    Trans,
    Super,
    Ignored
};

/// Offset or the user signals
constexpr SignalId INT_SIGNAL_START = static_cast<SignalId>(4);
constexpr SignalId EXT_SIGNAL_START = static_cast<SignalId>(32);

// state-handler function
class StateMachine;
using State = HandleResult (StateMachine::*) (const std::shared_ptr<const Event>&);

//////////////////////////////////////////////////////
//                                                  //
//////////////////////////////////////////////////////

class StateMachine {
    public:        
        //using State = HandleResult (StateMachine::*) (const std::shared_ptr<const Event>&);
        
        StateMachine() : _currentState{&StateMachine::top} {}
        virtual ~StateMachine() {};


        void init(void);
        void dispatch(const std::shared_ptr<const Event>& event);
        bool isIn(const State);

    protected:
        State getState(void) const { return _currentState; }

        HandleResult top(const std::shared_ptr<const Event>& event) { return HandleResult::Ignored; }

        static inline HandleResult retHandled(void) { return HandleResult::Handled; }
        static inline HandleResult retUnhandled(void) { return HandleResult::Unhandled; }   // because of a guard
        HandleResult retTrans(const State target);
        HandleResult retSuper(const State superState);

        enum ReservedStateMachineSignals : SignalId
        {
            ENTRY_SIG = static_cast<SignalId>(1),       //< signal for entry actions
            EXIT_SIG,                                   //< signal for exit actions
            INIT_SIG                                    //< signal for nested initial transitions
        };

        virtual HandleResult _topInitialTrans(void) = 0;

        template<typename T>
        constexpr HandleResult trans(T state) {
            return retTrans(static_cast<State>(state));
        }

        template<typename T>
        constexpr HandleResult super(T state) {
            return retSuper(static_cast<State>(state));
        }

    private:
        State _currentState;        //< current active state (state-variable)
        State _tempTargetState;     //< temporary state: target of transition or superstate

        State _procInitTrans(void);
};


} // namespace STATE_MACHINE_H

#endif