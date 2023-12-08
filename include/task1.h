#ifndef TASK1_H
#define TASK1_H

#include "ulak.h"

class Task1 : public uOS::Task {
    public:
        Task1() = delete;
        Task1(const uOS::TaskId, const std::string&);   
    
    private:
        void _stateMachine(std::shared_ptr<const uOS::Event>&) override;
        void _init(void) override;

        uint32_t _count {0};
};

#endif