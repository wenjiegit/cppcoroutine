#include "cppco.h"
#include <fcntl.h>
#include <stdio.h>
#include <list>

namespace cpp_coroutine {
bool _init = false;
std::shared_ptr<task_coroutine> _s_corouting_ptr = nullptr;

int task_main(std::function<void()> main_func_obj) {
    if (_init) {
        return 0;
    }

    _s_corouting_ptr = std::make_shared<task_coroutine>();
    _s_corouting_ptr->taskcreate(main_func_obj);
    _s_corouting_ptr->schedule();

    _init = true;
    return 0;
}

void coroutine_create(std::function<void()> func_obj) {
    _s_corouting_ptr->taskcreate(func_obj);
    return;
}

void coroutine_sleep(unsigned long long ms) {
    _s_corouting_ptr->co_sleep(ms);
}

std::shared_ptr<task_coroutine> get_coroutine() {
    return _s_corouting_ptr;
}

}

