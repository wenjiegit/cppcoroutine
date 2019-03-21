#ifndef TASK_COROUTINE_H
#define TASK_COROUTINE_H
#include "task_pub.h"

#include <thread>
#include <mutex>
#include <condition_variable>
#include <list>
#include <stdarg.h>
#include <functional>

#define STACK_DEF_SIZE (256*1024)

namespace cpp_coroutine {

class task_coroutine {
public:
    friend void taskstart(unsigned int y, unsigned int x);

    task_coroutine();
    ~task_coroutine();
    
    void run();
    int taskcreate(std::function<void()> func_obj, uint stack=STACK_DEF_SIZE);
    int taskyield(void);
    void schedule();

private:
    void contextswitch(Context *from, Context *to);
    void taskexit(Task_S *t);
    Task_S* taskalloc(std::function<void()> func_obj, uint stack);
    void taskswitch();
    void needstack(int n);
    void taskready(Task_S* t);
    bool task_list_empty();
    void taskstate(const char *fmt, ...);

private:
    std::shared_ptr<std::thread> _thread_ptr;
    std::list<Task_S*> _task_list;
    Task_S* _taskrunning;
    int	_taskcount;
    int _tasknswitch;
    int _taskidgen;
    Context	_taskschedcontext;

    std::mutex _mutex;
    std::condition_variable_any _noempty_cond;
};

}

#endif //TASK_COROUTINE_H