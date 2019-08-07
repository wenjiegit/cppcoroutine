#ifndef NET_COROUTINE_H
#define NET_COROUTINE_H
#include "net_base.h"
#include "task_coroutine.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/poll.h>
#include <string>

namespace cpp_coroutine {

class listen_coroutine : public net_listen {
public:
    listen_coroutine();
    virtual ~listen_coroutine();

    //createfd shall be called before others function.
    int createfd(bool is_tcp, std::string hostip, int port);

    //before call accept_conn, should call createfd to get fd.
    virtual std::shared_ptr<net_conn> accept_conn();
    virtual void close_conn();
    virtual ADDR_INFO get_addr();

private:
    void accept_wait();

private:
    int _fd;
    int _current_epoll_state;
    ADDR_INFO _info;
};

class net_coroutine: public net_conn {
public:
    net_coroutine(int fd);
    net_coroutine(int fd, ADDR_INFO remote, ADDR_INFO local);
    virtual ~net_coroutine();

    virtual int read_data(char* data_p, int data_size);
    virtual int write_data(char* data_p, int data_size);

    virtual int close_conn();
    virtual ADDR_INFO local_addr();
    virtual ADDR_INFO remote_addr();
    virtual int get_fd() {
        return _fd;
    }

private:
    void read_wait();
    void write_wait();
    void remove_epoll_event(int event_bits);

private:
    int _fd;
    int _current_epoll_state;
    ADDR_INFO _local_info;
    ADDR_INFO _remote_info;
};

}

#endif //NET_COROUTINE_H