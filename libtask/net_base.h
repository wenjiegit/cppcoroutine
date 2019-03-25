#ifndef NET_BASE_H
#define NET_BASE_H
#include <string>

namespace cpp_coroutine {
typedef  struct {
    std::string ip;
    int port;
}ADDR_INFO;

class net_listen {
public:
    virtual int accept_conn() = 0;
    virtual void close_conn() = 0;
    virtual ADDR_INFO get_addr() = 0;
};

class net_conn {
public:
    virtual int read_data(unsigned char* data_p, int data_size) = 0;
    virtual int write_data(unsigned char* data_p, int data_size) = 0;

    virtual int close_conn() = 0;
    virtual ADDR_INFO local_addr() = 0;
    virtual ADDR_INFO remote_addr() = 0;
};
}
#endif //NET_BASE_H