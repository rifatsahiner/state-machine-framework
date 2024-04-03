
#ifndef SIMPLE_TASK_H
#define SIMPLE_TASK_H

#include "uOS.h"

using namespace uOS;


namespace App1 {


class SimpleTask : public Task {
    public:
        SimpleTask(const TaskId, std::string&&);   

    private:
        uint16_t _counter{0};

        HandleResult _topInitialTrans(void) override;
        HandleResult singleState(const std::shared_ptr<const Event>&);
};


}   // namespace App1


#endif  // SIMPLE_TASK_H
