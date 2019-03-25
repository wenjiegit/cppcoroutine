#ifndef TASK_BASE_H
#define TASK_BASE_H
#include "task_coroutine.h"
#include <vector>

namespace cpp_coroutine {
class cppco;

using running_func = void (*) (void*, std::shared_ptr<task_coroutine>);

void run_task();

class cppco {
public:
    static int task_init();
    static void task_schedule();
    
    static int coroutine_create(running_func func_pm, void* param_p) {

        int random_index = rand() % _coroutine_ptr_vec.size();

        auto coroutine_obj = _coroutine_ptr_vec[random_index];

        auto usua_func_obj = std::bind(func_pm, param_p, coroutine_obj);
        
        coroutine_obj->taskcreate(usua_func_obj);
        return random_index;
    }

private:
    static std::vector<std::shared_ptr<task_coroutine>> _coroutine_ptr_vec;
};

}

#endif //TASK_BASE_H