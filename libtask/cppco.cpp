#include "cppco.h"
#include <fcntl.h>
#include <stdio.h>
#include <list>

namespace cpp_coroutine {
std::vector<std::shared_ptr<task_coroutine>> cppco::_coroutine_ptr_vec;

int cppco::task_init() {
    int proc_max = std::thread::hardware_concurrency() + 1;

    for (int i = 0; i < proc_max - 1; i++) {
        auto coroutine_ptr = std::make_shared<task_coroutine>();

        coroutine_ptr->run();
        _coroutine_ptr_vec.push_back(coroutine_ptr);
        coroutine_ptr->set_thread_index(_coroutine_ptr_vec.size()-1);
    }

    return 0;
}

void cppco::task_schedule() {
    auto coroutine_ptr = std::make_shared<task_coroutine>();

    _coroutine_ptr_vec.push_back(coroutine_ptr);
    coroutine_ptr->set_thread_index(_coroutine_ptr_vec.size()-1);
    coroutine_ptr->schedule();

    return;
}

}

