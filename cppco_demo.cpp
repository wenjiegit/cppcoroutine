#include "libtask/cppco.h"

using namespace cpp_coroutine;

void demo_func(void* index, std::shared_ptr<task_coroutine> task_co_ptr) {
    printf("demo_func index=%d, thread index=%d\r\n", *(int*)index, task_co_ptr->get_thread_index());

    return;
}

int main(int argn, char** argv) {
   
    cppco::task_init();
    int param_list[500];

    for (int index = 0; index < 500; index++) {
        param_list[index] = index;
        cppco::coroutine_create(demo_func, &param_list[index]);
    }

    cppco::task_schedule();

    return 0;
}