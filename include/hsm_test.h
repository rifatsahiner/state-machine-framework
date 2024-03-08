#ifndef HSM_TEST_H
#define HSM_TEST_H

#include "uOS.h"


namespace App1 {


class HsmTest : public uOS::Task {
    public:
        HsmTest(const uOS::TaskId, const std::string&);   

    private:
        uint8_t _foo;

        uOS::HandleResult _topInitialTrans(void) override;
        uOS::HandleResult s(const std::shared_ptr<const uOS::Event>&);
        uOS::HandleResult s1(const std::shared_ptr<const uOS::Event>&);
        uOS::HandleResult s11(const std::shared_ptr<const uOS::Event>&);
        uOS::HandleResult s2(const std::shared_ptr<const uOS::Event>&);
        uOS::HandleResult s21(const std::shared_ptr<const uOS::Event>&);
        uOS::HandleResult s211(const std::shared_ptr<const uOS::Event>&);
};


}   // namespace App1

#endif
