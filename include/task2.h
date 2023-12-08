#ifndef TASK2_H
#define TASK2_H

#include "ulak.h"

class Task2 : public uOS::Task {
    public:
        Task2() = delete;
        Task2(const uOS::TaskId, const std::string&);   
    
    private:
        void _stateMachine(std::shared_ptr<const uOS::Event>&) override;
        void _init(void) override;

        uint32_t _count {0};
};

#endif