#ifndef TASK3_H
#define TASK3_H

#include "ulak.h"

class Task3 : public uOS::Task {
    public:
        Task3() = delete;
        Task3(const uOS::TaskId, const std::string&);   
    
    private:
        void _stateMachine(std::shared_ptr<const uOS::Event>&) override;
        void _init(void) override;

        uint32_t _count {0};
};

#endif