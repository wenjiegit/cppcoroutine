#include "cppco.h"
#include <fcntl.h>
#include <stdio.h>
#include <list>

namespace cpp_coroutine {
std::vector<std::shared_ptr<task_coroutine>> cppco::_coroutine_ptr_vec;
std::mutex cppco::_mutex;
bool cppco::_init = false;

int cppco::task_init() {
    std::lock_guard<std::mutex> locker(_mutex);
    if (_init) {
        return 0;
    }

    int proc_max = std::thread::hardware_concurrency() + 1;

    for (int i = 0; i < proc_max - 1; i++) {
        auto coroutine_ptr = std::make_shared<task_coroutine>();

        coroutine_ptr->run();
        _coroutine_ptr_vec.push_back(coroutine_ptr);
        coroutine_ptr->set_thread_index(_coroutine_ptr_vec.size()-1);
    }

    _init = true;
    return 0;
}

void cppco::task_schedule() {
    task_init();
    auto coroutine_ptr = std::make_shared<task_coroutine>();

    _coroutine_ptr_vec.push_back(coroutine_ptr);
    coroutine_ptr->set_thread_index(_coroutine_ptr_vec.size()-1);
    coroutine_ptr->schedule();

    return;
}

}

