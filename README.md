# state-machine-framework
A full-fledged UML state-machine framework with support for hierarchical (nested) states   

- Divide and conquire with encapsulated 'tasks' that are constructed as hierarchical state machines
- Dynamic task creation and related thread management
- Zero-copy event delivery system with thread safe event queue structures
- Timed event delivery
- Publish-subscribe communication
- System-wide state monitoring with comprehensive TUI

Inspired and based on 2008 book of "Practical UML Statecharts in C/C++" which is intended for embedded systems.   
This implementation of UML state-machine framework however is intended for high level server side applications and written in modern C++17

- better encapsulation, only with task ids
