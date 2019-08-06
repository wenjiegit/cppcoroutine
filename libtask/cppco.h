#ifndef TASK_BASE_H
#define TASK_BASE_H
#include "task_coroutine.h"
#include <vector>

namespace cpp_coroutine {
int task_main(std::function<void()> main_func_obj);

void coroutine_create(std::function<void()> func_obj);

void coroutine_sleep(unsigned long long ms);
}

#endif //TASK_BASE_H