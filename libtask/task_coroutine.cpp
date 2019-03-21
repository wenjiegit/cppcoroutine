#include "task_coroutine.h"
#include <stdarg.h>

namespace cpp_coroutine {

void taskstart(unsigned int y, unsigned int x)
{
	Task_S *t;
	unsigned long z;

	z = x<<16;	/* hide undefined 32-bit shift from 32-bit compilers */
	z <<= 16;
	z |= y;
	t = (Task_S*)z;

//print("taskstart %p\n", t);
	t->func_obj();
    task_coroutine* co_p = (task_coroutine*)(t->co_p);

//print("taskexits %p\n", t);
	co_p->taskexit(t);
//print("not reacehd\n");
}

task_coroutine::task_coroutine():_taskrunning(nullptr)
    ,_taskcount(0)
    ,_tasknswitch(0)
    ,_taskidgen(0) {

}

task_coroutine::~task_coroutine() {

}

void task_coroutine::taskexit(Task_S *t)
{
	_taskrunning->exiting = 1;
	taskswitch();
}

void task_coroutine::needstack(int n) {
	Task_S *t;

	t = _taskrunning;

	if((char*)&t <= (char*)t->stk
	|| (char*)&t - (char*)t->stk < 256+n){
		abort();
	}
}

void task_coroutine::taskswitch() {
	needstack(0);
	contextswitch(&_taskrunning->context, &_taskschedcontext);
}

void task_coroutine::run() {
    _thread_ptr = std::make_shared<std::thread>(std::thread(&task_coroutine::schedule, this));
}

Task_S* task_coroutine::taskalloc(std::function<void()> func_obj, unsigned int stack) {
	Task_S *t;
	sigset_t zero;
	unsigned int x, y;
	unsigned long z;
    const int reserve = 256;

	/* allocate the task and stack together */
	t = (Task_S*)malloc(sizeof *t + reserve + stack);
	if(t == nullptr){
		abort();
	}
	memset(t, 0, sizeof *t);
	t->stk = ((unsigned char*)(t+1) + reserve);
	t->stksize = stack;
	t->id = ++_taskidgen;
	t->func_obj = func_obj;
    t->co_p = (void*)this;

	/* do a reasonable initialization */
	memset(&t->context.uc, 0, sizeof t->context.uc);
	sigemptyset(&zero);
	sigprocmask(SIG_BLOCK, &zero, &t->context.uc.uc_sigmask);

	/* must initialize with current context */
	if(getcontext(&t->context.uc) < 0){
		abort();
	}

	/* call makecontext to do the real work. */
	/* leave a few words open on both ends */
	t->context.uc.uc_stack.ss_sp = t->stk+8;
	t->context.uc.uc_stack.ss_size = t->stksize-64;
#if defined(__sun__) && !defined(__MAKECONTEXT_V2_SOURCE)		/* sigh */
#warning "doing sun thing"
	/* can avoid this with __MAKECONTEXT_V2_SOURCE but only on SunOS 5.9 */
	t->context.uc.uc_stack.ss_sp = 
		(char*)t->context.uc.uc_stack.ss_sp
		+t->context.uc.uc_stack.ss_size;
#endif
	/*
	 * All this magic is because you have to pass makecontext a
	 * function that takes some number of word-sized variables,
	 * and on 64-bit machines pointers are bigger than words.
	 */
//print("make %p\n", t);
	z = (unsigned long)t;
	y = z;
	z >>= 16;	/* hide undefined 32-bit shift from 32-bit compilers */
	x = z>>16;
	makecontext(&t->context.uc, (void(*)())taskstart, 2, y, x);

	return t;
}

int task_coroutine::taskcreate(std::function<void()> func_obj, uint stack) {
	int id;
	Task_S *t;

	t = taskalloc(func_obj, stack);
	_taskcount++;
	id = t->id;

	taskready(t);

	return id;
}

bool task_coroutine::task_list_empty() {
    std::lock_guard<std::mutex> locker(_mutex);

    bool ret = _task_list.empty();
    
    if (ret || (_taskcount <= 0)) {
        _noempty_cond.wait(_mutex);
    }
    
    return ret;
}

void task_coroutine::taskready(Task_S* t) {
    std::lock_guard<std::mutex> locker(_mutex);

	t->ready = 1;
	_task_list.push_back(t);
    _noempty_cond.notify_one();
}

void task_coroutine::schedule() {
    while(true) {
		task_list_empty();

        auto first_task = _task_list.front();
        _task_list.pop_front();
        first_task->ready = 0;

        _taskrunning = first_task;
        contextswitch(&_taskschedcontext, &first_task->context);

        _taskrunning = nullptr;
		if(first_task->exiting){
			if(!first_task->system)
				_taskcount--;
			free(first_task);
		}
    };
}

int task_coroutine::taskyield(void) {
	int n;
	
	n = _tasknswitch;
	taskready(_taskrunning);
	taskstate("yield");
	taskswitch();
	return _tasknswitch - n - 1;
}

void task_coroutine::contextswitch(Context *from, Context *to){
	if(swapcontext(&from->uc, &to->uc) < 0){
		assert(0);
	}
}

void task_coroutine::taskstate(const char *fmt, ...)
{
	va_list arg;
	Task_S *t;

	t = _taskrunning;
	va_start(arg, fmt);
	vsprintf(t->state, fmt, arg);
	va_end(arg);
}

}