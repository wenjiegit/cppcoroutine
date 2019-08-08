#include "coroutine/cppco.h"
#include "coroutine/net_base.h"
#include "net_coroutine.h"
#include "colog/colog.h"

#include <string>

using namespace cpp_coroutine;

#define BUFFER_SIZE 1024

void on_echo_work(std::shared_ptr<net_conn> net_conn_obj);

void net_co_start_entry(int port) {
    CO_LOGF(LOG_INFO, "net start listen port:%d", port);
    net_init();

    std::shared_ptr<listen_coroutine> tcp_listen_ptr = std::make_shared<listen_coroutine>();
    int ret_fd = tcp_listen_ptr->createfd(true, "0.0.0.0", 7000);
    if (ret_fd <= 0) {
        CO_LOG(LOG_INFO, "create fd error....");
        exit(0);
    }

    while(true) {
        auto net_conn_obj = tcp_listen_ptr->accept_conn();
        if (!net_conn_obj) {
            CO_LOG(LOG_INFO, "tcp accept error, exist...");
            break;
        }
        coroutine_create(std::bind(on_echo_work, net_conn_obj));
    };

    return;
}

void on_echo_work(std::shared_ptr<net_conn> net_conn_obj) {
    char buffer[BUFFER_SIZE];

    memset(buffer, 0, BUFFER_SIZE);

    //CO_LOGF(LOG_INFO, "on_echo_work hostip:%s, port:%d, fd:%d", 
    //    net_conn_obj->remote_addr().ip.c_str(), net_conn_obj->remote_addr().port,
    //    net_conn_obj->get_fd());
    
    int read_len = net_conn_obj->read_data(buffer, BUFFER_SIZE);
    if (read_len <= 0) {
        net_conn_obj->close_conn();
        return;
    }
    //CO_LOGF(LOG_INFO, "read data:%s, len:%d, fd:%d", 
    //    buffer, read_len, net_conn_obj->get_fd());
    int write_len = net_conn_obj->write_data(buffer, read_len);
    if (write_len <= 0) {
        net_conn_obj->close_conn();
        return;
    }
    //CO_LOGF(LOG_INFO, "write data:%s, len:%d, fd:%d, use_count:%ld", 
    //    buffer, write_len, net_conn_obj->get_fd(), net_conn_obj.use_count());
    
    return;
}

int main(int argn ,char** argv) {
    if (argn != 2) {
        CO_LOG(LOG_INFO, "it should be ./tcp_server_demo tcpport");
        return 0;
    }
    int port = atoi(argv[1]);

    LOG_INIT("./colog.log");
    task_main(std::bind(net_co_start_entry, port));

    return 1;
}
