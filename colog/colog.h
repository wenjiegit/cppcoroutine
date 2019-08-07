#ifndef COLOG_H
#define COLOG_H
#include <atomic>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <thread>
#include <fstream>
#include <string>
#include <stdarg.h>

namespace cpp_coroutine {

#define LOG_DEBUG 0
#define LOG_INFO  1
#define LOG_WARN  2
#define LOG_ERROR 3

typedef struct {
    int log_level;
    std::string info;
    std::string filename;
    int line;
} LOG_INFO_S;

class co_log {
public:
    static co_log* get_instance();

    ~co_log();

    void start();
    void stop();

    void set_logfilename(std::string filename);
    void set_loglevel(int log_level);
    int get_loglevel();

    int logf(int log_level, std::string info, std::string filename, unsigned int line);

private:
    co_log();
    void on_work();
    void get_log(LOG_INFO_S& ret_info);

private:
    static std::atomic<co_log*> _instance;
    static std::mutex _s_mutex;
    static int _log_level;

    std::queue<LOG_INFO_S> _queue;
    std::mutex _mutex;
    std::condition_variable_any _notify_cond;
    std::string _filename;
    std::shared_ptr<std::thread> _thread_ptr;
    bool _running_flag;
};

#define LOG_INIT(filename) \
    co_log::get_instance()->set_logfilename(filename); \
    co_log::get_instance()->start(); \

#define LOG_DEINIT()                \
    co_log::get_instance()->stop(); \

#define CO_LOGF(log_level, fmt, ...) \
{ \
    if (co_log::get_instance()->get_loglevel() <= log_level)                 \
    {                                                                        \
        char buffer[512];                                                    \
        sprintf(buffer, fmt, __VA_ARGS__);                                   \
        co_log::get_instance()->logf(log_level, buffer, __FILE__, __LINE__); \   
    }                                                                        \
} \

#define CO_LOG(log_level, fmt) \
{ \
    if (co_log::get_instance()->get_loglevel() <= log_level)                 \
    {                                                                        \
        co_log::get_instance()->logf(log_level, fmt, __FILE__, __LINE__); \   
    }                                                                        \
} \

};
#endif //COLOG_H