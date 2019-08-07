#ifndef NET_BASE_H
#define NET_BASE_H
#include "colog/colog.h"
#include <string>
#include <memory>

#define INVALID_SOCKET (-1)

#define READ_WAIT 0x01
#define WRITE_WAIT 0x02

#define OP_ADD 0x01
#define OP_MOD 0x02
#define OP_DEL 0x04

namespace cpp_coroutine {
typedef  struct {
    std::string ip;
    int port;
}ADDR_INFO;

class net_conn;

class net_listen {
public:
    net_listen(){};
    virtual ~net_listen() {};
    virtual std::shared_ptr<net_conn> accept_conn() = 0;
    virtual void close_conn() = 0;
    virtual ADDR_INFO get_addr() = 0;
};

class net_conn {
public:
    net_conn(){};
    virtual ~net_conn(){
        //CO_LOG(LOG_INFO, "destruct net conn...");
    };
    virtual int read_data(char* data_p, int data_size) = 0;
    virtual int write_data(char* data_p, int data_size) = 0;
    virtual int get_fd() = 0;

    virtual int close_conn() = 0;
    virtual ADDR_INFO local_addr() = 0;
    virtual ADDR_INFO remote_addr() = 0;
};

int parseip(const std::string& hostname, uint32_t *ip);
int netlookup(const std::string& hostname, unsigned int *ip);

void net_init();
int fdnoblock(int fd);

}
#endif //NET_BASE_H