#include "libtask/cppco.h"
#include "libtask/net_base.h"
#include "net_coroutine.h"
#include <string>

using namespace cpp_coroutine;

#define BUFFER_SIZE 1024

void on_echo_work(std::shared_ptr<net_conn> net_conn_obj);

void net_co_start_entry(int port) {
    printf("net start listen port:%d\r\n", port);
    net_init();

    std::shared_ptr<listen_coroutine> tcp_listen_ptr = std::make_shared<listen_coroutine>();
    int ret_fd = tcp_listen_ptr->createfd(true, "127.0.0.1", 7000);
    if (ret_fd <= 0) {
        printf("create fd error....\r\n");
        exit(0);
    }

    while(true) {
        auto net_conn_obj = tcp_listen_ptr->accept_conn();
        if (!net_conn_obj) {
            printf("tcp accept error, exist...\r\n");
            break;
        }
        coroutine_create(std::bind(on_echo_work, net_conn_obj));
    };

    return;
}

void on_echo_work(std::shared_ptr<net_conn> net_conn_obj) {
    char buffer[BUFFER_SIZE];

    printf("on_echo_work hostip:%s, port:%d, fd:%d\r\n", 
        net_conn_obj->remote_addr().ip.c_str(), net_conn_obj->remote_addr().port,
        net_conn_obj->get_fd());
    
    int read_len = net_conn_obj->read_data(buffer, BUFFER_SIZE);
    if (read_len <= 0) {
        net_conn_obj->close_conn();
        return;
    }
    printf("read data:%s, len:%d, fd:%d\r\n", buffer, read_len, net_conn_obj->get_fd());
    int write_len = net_conn_obj->write_data(buffer, read_len);
    if (write_len <= 0) {
        net_conn_obj->close_conn();
        return;
    }
    printf("write data:%s, len:%d, fd:%d, use_count:%ld\r\n", 
        buffer, write_len, net_conn_obj->get_fd(), net_conn_obj.use_count());
    
    return;
}

int main(int argn ,char** argv) {
    if (argn != 2) {
        printf("it should be ./tcp_server_demo tcpport\r\n");
        return 0;
    }
    int port = atoi(argv[1]);
    task_main(std::bind(net_co_start_entry, port));

    return 1;
}
