#ifndef TASK_BASE_H
#define TASK_BASE_H
#include "task_coroutine.h"
#include <vector>

namespace cpp_coroutine {

void run_task();

class task_base {
public:
    static int task_init();
    static void task_schedule();
    
    template<class F, class... Args>
    static int coroutine_create(F f, Args... args) {
        auto input_func_obj = std::bind(f, args...);

        auto usua_func_obj = [input_func_obj]{
            input_func_obj();
        };

        int random_index = rand() % _coroutine_ptr_vec.size();

        auto coroutine_obj = _coroutine_ptr_vec[random_index];

        coroutine_obj->taskcreate(usua_func_obj);
        return random_index;
    }

private:
    static std::vector<std::shared_ptr<task_coroutine>> _coroutine_ptr_vec;
};

}

#endif //TASK_BASE_H