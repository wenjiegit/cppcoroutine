#include "libtask/cppco.h"

using namespace cpp_coroutine;

void demo_func(void* index, std::shared_ptr<task_coroutine> task_co_ptr) {
    unsigned long long sleep_ms = (unsigned long long)(*(int*)index + 1000);

    if ((*(int*)index % 2) == 0) {
        sleep_ms = (unsigned long long)(*(int*)index + 1000);
    } else {
        sleep_ms = (unsigned long long)(*(int*)index + 3000);
    }
    
    task_co_ptr->co_sleep(sleep_ms);
    printf("demo_func index=%d, sleep_ms=%u, thread index=%d\r\n", 
        *(int*)index, sleep_ms, task_co_ptr->get_thread_index());
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