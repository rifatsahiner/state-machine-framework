
#include "App1.h"
#include "hsm_test.h"
#include <iostream>


namespace App1 {


HsmTest::HsmTest(const uOS::TaskId taskId, const std::string& taskName) : Task (taskId, taskName) {}

uOS::HandleResult HsmTest::_topInitialTrans(void){
    _foo = 0;
    std::cout << "top-INIT;";
    return trans(&HsmTest::s2);
}

uOS::HandleResult HsmTest::s(const std::shared_ptr<const uOS::Event>& event){
    uOS::HandleResult res;

    switch (event->signal) {
        case ENTRY_SIG: {
            std::cout << "s-ENTRY;";
            res = handled();
            break;
        }
        case EXIT_SIG: {
            std::cout << "s-EXIT;";
            res = handled();
            break;
        }
        case INIT_SIG: {
            std::cout << "s-INIT;";
            res = trans(&HsmTest::s11);
            break;
        }
        case SIGNAL_E : {
            std::cout << "s-E;";
            res = trans(&HsmTest::s11);
            break;
        }
        case SIGNAL_I : {
            if(_foo == 1){
                _foo = 0;
                std::cout << "s-I;";
                res = handled();
            } else {
                res = unhandled();
            }
            break;
        }
        case App1::AppSignals::TERMINATE : {
            // kill application
            std::cout << "s-Terminate";
            uOS::FW::stop();
            res = super(&HsmTest::top);
            break;
        }
        default: {
            res = super(&HsmTest::top);
            break;
        }
    }
    return res;
}
        
uOS::HandleResult HsmTest::s1(const std::shared_ptr<const uOS::Event>& e){    
    uOS::HandleResult r;

    switch (e->signal) {
        case ENTRY_SIG: {
            std::cout << "s1-ENTRY;";
            r = handled();
            break;
        }
        case EXIT_SIG: {
            std::cout << "s1-EXIT;";
            r = handled();
            break;
        }
        case INIT_SIG: {
            std::cout << "s1-INIT;";
            r = trans(&HsmTest::s11);
            break;
        }
        case SIGNAL_A: {
            std::cout << "s1-A;";
            r = trans(&HsmTest::s1);
            break;
        }
        case SIGNAL_B: {
            std::cout << "s1-B;";
            r = trans(&HsmTest::s11);
            break;
        }
        case SIGNAL_C: {
            std::cout << "s1-C;";
            r = trans(&HsmTest::s2);
            break;
        }
        case SIGNAL_D: {
            if(_foo == 0){
                _foo = 1;
                std::cout << "s1-D;";
                r = trans(&HsmTest::s);
            } else {
                r = unhandled();
            }
            break;
        }
        case SIGNAL_F: {
            std::cout << "s1-F;";
            r = trans(&HsmTest::s211);
            break;
        }
        case SIGNAL_I: {
            std::cout << "s1-I;";
            r = handled();
            break;
        }
        default: {
            r = super(&HsmTest::s);
            break;
        }
    }
    return r;
}

uOS::HandleResult HsmTest::s11(const std::shared_ptr<const uOS::Event>& e)
{
    switch (e->signal)
    {
        case ENTRY_SIG: {
            std::cout << "s11-ENTRY;";
            return handled();
        }
        case EXIT_SIG: {
            std::cout << "s11-EXIT;";
            return handled();
        }
        case SIGNAL_D: {
            if(_foo == 1){
                _foo = 0;
                std::cout << "s11-D;";
                return trans(&HsmTest::s1);
            } else {
                return unhandled();
            }
        }
        case SIGNAL_G: {
            std::cout << "s11-G;";
            return trans(&HsmTest::s211);
        }
        case SIGNAL_H: {
            std::cout << "s11-H;";
            return trans(&HsmTest::s);
        }
        default:
            return super(&HsmTest::s1);
    }
}

uOS::HandleResult HsmTest::s2(const std::shared_ptr<const uOS::Event>& e)
{
    switch (e->signal)
    {
        case ENTRY_SIG: {
            std::cout << "s2-ENTRY;";
            return handled();
        }
        case EXIT_SIG: {
            std::cout << "s2-EXIT;";
            return handled();
        }
        case INIT_SIG: {
            std::cout << "s2-INIT;";
            return trans(&HsmTest::s211);
        }
        case SIGNAL_C: {
            std::cout << "s2-C;";
            return trans(&HsmTest::s1);
        }
        case SIGNAL_F: {
            std::cout << "s2-F;";
            return trans(&HsmTest::s11);
        }
        case SIGNAL_I: {
            if(_foo == 0){
                _foo = 1;
                std::cout << "s2-I;";
                return handled();
            } else {
                return unhandled();
            }
        }
        default: {
            return super(&HsmTest::s);
        }

    }
}

uOS::HandleResult HsmTest::s21(const std::shared_ptr<const uOS::Event>& e)
{
    switch (e->signal)
    {
        case ENTRY_SIG: {
            std::cout << "s21-ENTRY;";
            return handled();
        }
        case EXIT_SIG: {
            std::cout << "s21-EXIT;";
            return handled();
        }
        case INIT_SIG: {
            std::cout << "s21-INIT;";
            return trans(&HsmTest::s211);
        }
        case SIGNAL_A: {
            std::cout << "s21-A;";
            return trans(&HsmTest::s21);
        }
        case SIGNAL_B: {
            std::cout << "s21-B;";
            return trans(&HsmTest::s211);
        }
        case SIGNAL_G: {
            std::cout << "s21-G;";
            return trans(&HsmTest::s11);
        }
        default: {
            return super(&HsmTest::s2);
        }
    }
}

uOS::HandleResult HsmTest::s211(const std::shared_ptr<const uOS::Event>& e)
{
    switch (e->signal)
    {
        case ENTRY_SIG: {
            std::cout << "s211-ENTRY;";
            return handled();
        }
        case EXIT_SIG: {
            std::cout << "s211-EXIT;";
            return handled();
        }
        case SIGNAL_D: {
            std::cout << "s211-D;";
            return trans(&HsmTest::s21);
        }
        case SIGNAL_H: {
            std::cout << "s211-H;";
            return trans(&HsmTest::s);
        }
        default: {
            return super(&HsmTest::s21);
        }
    }
}


}   // namespace App1
