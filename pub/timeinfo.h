#ifndef TIME_INFO_H
#define TIME_INFO_H
#include <chrono>
#include <string>

namespace cpp_coroutine {
inline long long now_timestamp_ms() {
    std::chrono::system_clock::duration d = std::chrono::system_clock::now().time_since_epoch();

    std::chrono::milliseconds mil = std::chrono::duration_cast<std::chrono::milliseconds>(d);

    return mil.count();
}

inline long long now_timestamp_micsec() {
    std::chrono::steady_clock::duration d = std::chrono::steady_clock::now().time_since_epoch();

    std::chrono::microseconds mil = std::chrono::duration_cast<std::chrono::microseconds>(d);

    return mil.count();
}

inline std::string get_now_string() {
    char some_buffer[80];
    auto t = std::chrono::system_clock::now();
    auto as_time_t = std::chrono::system_clock::to_time_t(t); 

    std::chrono::steady_clock::duration d = t.time_since_epoch();
    std::chrono::milliseconds mil = std::chrono::duration_cast<std::chrono::milliseconds>(d);

    struct tm tm; 
    if (::gmtime_r(&as_time_t, &tm)) 
    if (std::strftime(some_buffer, sizeof(some_buffer), "%D:%H:%M", &tm))
        sprintf(some_buffer, "%s:%ld", some_buffer, mil.count()%1000); 
        return std::string{some_buffer}; 
    throw std::runtime_error("Failed to get current date as string"); 
}

};

#endif //TIME_INFO_H