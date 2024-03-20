#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include <memory>

#include "uOS_defs.h"


namespace uOS {


enum class HandleResult : uint_fast8_t {
    Handled,
    Unhandled,
    Trans,
    Super,
    Ignored
};

// definition for state-handler function
class StateMachine;
using State = HandleResult (StateMachine::*) (const std::shared_ptr<const Event>&);

//////////////////////////////////////////////////////
//                                                  //
//////////////////////////////////////////////////////

class StateMachine {
    public:        
        StateMachine() : _currentState{&StateMachine::top} {}
        virtual ~StateMachine() {};

        void init(void);    // todo: bunlar protected olacak
        void dispatch(const std::shared_ptr<const Event>& event);
        bool isIn(const State);

    protected:
        // ??
        State getState(void) const { return _currentState; }

        // conceptual UML top state that ignores all events
        HandleResult top(const std::shared_ptr<const Event>& event) {
            (void)event;
            return HandleResult::Ignored;
        }

        //
        // macro-like functions for event handle returns (handled/unhandled/transiton/superstate)
        //
        static constexpr HandleResult handled(void) { return HandleResult::Handled; }

        static constexpr HandleResult unhandled(void) { return HandleResult::Unhandled; }   // because of a guard

        template<typename T>
        constexpr HandleResult trans(T target) {
            static_assert(std::is_base_of_v<StateMachine, remove_ptr_to_member_func_t<T>>);
            
            // store target in temp and return result as state transition
            _tempTargetState = static_cast<State>(target);
            return HandleResult::Trans;
        }

        template<typename T>
        constexpr HandleResult super(T superstate) {
            static_assert(std::is_base_of_v<StateMachine, remove_ptr_to_member_func_t<T>>);
            
            // store superstate in temp and return result as delegated to superstate
            _tempTargetState = static_cast<State>(superstate);
            return HandleResult::Super;
        }

        // reserved signals accoridng to UML semantics
        enum ReservedSignals : SignalId {
            ENTRY_SIG = static_cast<SignalId>(1),       // signal for entry actions
            EXIT_SIG,                                   // signal for exit actions
            INIT_SIG                                    // signal for nested initial transitions
        };

    private:
        State _currentState;        // current active state (state-variable)
        State _tempTargetState;     // temporary state: target of transition or superstate

        State _procInitTrans(void);
        virtual HandleResult _topInitialTrans(void) = 0;

    private:
        // custom type trait that gets class from a pointer to member function in 'State' form
        // used for type checking state handler functions
        template <typename> struct remove_ptr_to_member_func;

        template <typename T>
        struct remove_ptr_to_member_func<HandleResult (T::*)(const std::shared_ptr<const Event>&)> {
            using type = T;
        };

        template<typename U>
        using remove_ptr_to_member_func_t = typename remove_ptr_to_member_func<U>::type;
};


} // namespace STATE_MACHINE_H

#endif