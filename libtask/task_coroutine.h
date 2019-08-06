#ifndef TASK_COROUTINE_H
#define TASK_COROUTINE_H
#include "task_pub.h"

#include <thread>
#include <mutex>
#include <condition_variable>
#include <list>
#include <stdarg.h>
#include <functional>
#include <map>

#define STACK_DEF_SIZE (256*1024)

namespace cpp_coroutine {

class task_coroutine;
class listen_coroutine;
class net_coroutine;

extern void net_co_onwork(void* param_p, std::shared_ptr<task_coroutine> self_co_ptr);
extern void net_init();
extern void net_run();

class task_coroutine {
public:
    friend class listen_coroutine;
    friend class net_coroutine;
    
    friend void taskstart(unsigned int y, unsigned int x);
    friend void net_co_onwork(void* param_p, std::shared_ptr<task_coroutine> self_co_ptr);
    friend void net_init();
    friend void net_run();

    task_coroutine();
    ~task_coroutine();

    //start a new thread to run schedule function which is one dead loop function to run new task function.
    void run();//not used in single system thread
    //schedule is one dead loop function to run new task function.
    void schedule();
    //sleep time to give up process
    unsigned long long co_sleep(unsigned long long sleep_ms);
    //create coroutine task for function running
    int taskcreate(std::function<void()> func_obj, uint stack=STACK_DEF_SIZE);
    Task_S* get_runing_task() {
        return _taskrunning;
    }

private:
    //sleeping function wakeup and taskready.
    void sleep_wakeup();

    int get_sleep_waitms();
    
    //add current running function to task list, give up running state by calling taskswitch()
    int taskyield(void);

    //call swapcontext to switch to a new function
    void contextswitch(Context *from, Context *to);

    //set exit bit, call taskswitch() to switch to a new function
    void taskexit();

    //malloc a Task_S struct and make context
    Task_S* taskalloc(std::function<void()> func_obj, uint stack);

    //switch to a new function
    void taskswitch();

    //check task stack whether it is valid.
    void needstack(int n);

    //set ready bit, and add it to task list.
    void taskready(Task_S* t);

    //check whether task list is empty
    bool task_list_empty();

    //set task state descr
    void taskstate(const char *fmt, ...);

private:
    std::shared_ptr<std::thread> _thread_ptr;
    std::list<Task_S*> _task_list;
    std::map<unsigned long long, Task_S*> _sleep_task_map;
    Task_S* _taskrunning;
    int	_taskcount;
    int _tasknswitch;
    int _taskidgen;
    Context	_taskschedcontext;
};

}

#endif //TASK_COROUTINE_H