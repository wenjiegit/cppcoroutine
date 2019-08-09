#include "colog.h"
#include "pub/timeinfo.h"

namespace cpp_coroutine {

const char* LOG_DEBUG_STR = "DEBUG";
const char* LOG_INFO_STR = "INFO";
const char* LOG_WARN_STR = "WARN";
const char* LOG_ERROR_STR = "ERROR";
const char* LOG_UNKNOWN_STR = "UNKNOWN";

const char* get_leve_dscr(int log_level) {
    switch(log_level) {
        case LOG_DEBUG:
            return LOG_DEBUG_STR;
        case LOG_INFO:
            return LOG_INFO_STR;
        case LOG_WARN:
            return LOG_WARN_STR;
        case LOG_ERROR:
            return LOG_ERROR_STR;
        default:
            return LOG_UNKNOWN_STR;
    }
}

std::atomic<co_log*> co_log::_instance;
std::mutex co_log::_s_mutex;
int co_log::_log_level = LOG_INFO;

co_log* co_log::get_instance() {
    co_log* temp = _instance.load(std::memory_order_relaxed);
    std::atomic_thread_fence(std::memory_order_acquire);
    if (temp == nullptr) {
        std::lock_guard<std::mutex> lock(_s_mutex);
        temp = _instance.load(std::memory_order_relaxed);
        if (temp == nullptr) {
            temp = new co_log();
            std::atomic_thread_fence(std::memory_order_release);
            _instance.store(temp, std::memory_order_relaxed);
        }
    }
    return temp;
}

co_log::co_log() {

}

co_log::~co_log() {

}

void co_log::start() {
    _running_flag = true;
    _thread_ptr = std::make_shared<std::thread>(std::bind(&co_log::on_work, this));
}

void co_log::stop() {
    _running_flag = false;
    _thread_ptr->join();
}

void co_log::set_loglevel(int log_level) {
    _log_level = log_level;
}

int co_log::get_loglevel() {
    return _log_level;
}

void co_log::set_logfilename(std::string filename) {
    _filename = filename;
}

int co_log::logf(int log_level, std::string info, std::string filename, unsigned int line) {
    std::lock_guard<std::mutex> lock(_mutex);
    LOG_INFO_S msg;
    msg.log_level = log_level;
    msg.info = info;
    msg.filename = filename;
    msg.line = line;
    msg.date_str = get_now_string();
    _queue.push(msg);

    int queue_len = _queue.size();
    _notify_cond.notify_one();

    return queue_len;
}

void co_log::get_log(LOG_INFO_S& ret_info) {
    std::lock_guard<std::mutex> lock(_mutex);
    while (_queue.empty()) {
        _notify_cond.wait(_mutex);
    }

    ret_info = _queue.front();
    _queue.pop();

    return;
}

void co_log::on_work() {
    char szInfo[1024];

    while(_running_flag) {
        LOG_INFO_S msg;
        get_log(msg);
        const char* log_level_sz = get_leve_dscr(msg.log_level);
        sprintf(szInfo, "[%s][%s:%d][%s]: %s", msg.date_str.c_str(),
                msg.filename.c_str(), msg.line, log_level_sz, msg.info.c_str());
        if (_filename.empty()) {
            printf(szInfo);
        } else {
            std::fstream output_file;
            output_file.open(_filename, std::ios::in|std::ios::out|std::ios::app);
            if (output_file.is_open()) {
                output_file << szInfo << std::endl;
                output_file.close();
            }
        }
    }
}
};