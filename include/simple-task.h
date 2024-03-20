
#ifndef SIMPLE_TASK_H
#define SIMPLE_TASK_H

#include "uOS.h"


namespace App1 {


class SimpleTask : public uOS::Task {
    public:
        SimpleTask(const uOS::TaskId, std::string&&);   

    private:
        uint16_t _counter{0};

        uOS::HandleResult _topInitialTrans(void) override;
        uOS::HandleResult singleState(const std::shared_ptr<const uOS::Event>&);
};


}   // namespace App1

#endif
