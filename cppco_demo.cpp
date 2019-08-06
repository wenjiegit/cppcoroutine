#include "libtask/cppco.h"

using namespace cpp_coroutine;

void demo_func(int index) {
    unsigned long long sleep_ms = (unsigned long long)(index + 1000);

    if ((index % 2) == 0) {
        sleep_ms = (unsigned long long)(index + 1000);
    } else {
        sleep_ms = (unsigned long long)(index + 3000);
    }
    
    coroutine_sleep(sleep_ms);
    printf("demo_func index=%d, sleep_ms=%u\r\n", index, sleep_ms);
    return;
}

void coroutine_start() {
    for (int index = 0; index < 500; index++) {
        coroutine_create(std::bind(demo_func, index));
    }
}

int main(int argn, char** argv) {
   
    task_main(coroutine_start);


    return 0;
}