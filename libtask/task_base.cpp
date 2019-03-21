#include "task_base.h"
#include <fcntl.h>
#include <stdio.h>
#include <list>

namespace cpp_coroutine {
std::vector<std::shared_ptr<task_coroutine>> task_base::_coroutine_ptr_vec;

int task_base::task_init() {
    int proc_max = std::thread::hardware_concurrency() + 1;

    for (int i = 0; i < proc_max - 1; i++) {
        auto coroutine_ptr = std::make_shared<task_coroutine>();

        coroutine_ptr->run();
        _coroutine_ptr_vec.push_back(coroutine_ptr);
    }

    return 0;
}

void task_base::task_schedule() {
    auto coroutine_ptr = std::make_shared<task_coroutine>();

    _coroutine_ptr_vec.push_back(coroutine_ptr);

    coroutine_ptr->schedule();

    return;
}

}

