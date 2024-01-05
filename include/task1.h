#ifndef TASK1_H
#define TASK1_H

#include "uOS.h"


namespace App1 {


class Task1 : public uOS::Task {
    public:
        Task1() = delete;
        Task1(const uOS::TaskId, const std::string&, const int& i, const float& f, const std::string& s);   
    
    protected:
        uOS::HandleResult _topInitialTrans(void) override;
        uOS::HandleResult state1(const std::shared_ptr<const uOS::Event>&);
        uOS::HandleResult state2(const std::shared_ptr<const uOS::Event>&);

    private:
        int _i;
        float _f;
        std::string _s;
};


}   // namespace App1


#endif
