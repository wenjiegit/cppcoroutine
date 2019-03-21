#include "libtask/task_base.h"

void demo_func(int index) {
    printf("demo_func index=%d\r\n", index);

    return;
}

int main(int argn, char** argv) {

    cpp_coroutine::task_base::task_init();

    for (int index = 0; index < 500; index++) {
        cpp_coroutine::task_base::coroutine_create(demo_func, index);
    }
    cpp_coroutine::task_base::task_schedule();

    return 0;
}