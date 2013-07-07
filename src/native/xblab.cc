#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <string>
#include <map>
#include <string>

#include "macros.h"
#include "server.h"
#include "manager.h"

namespace xblab {

uv_loop_t *loop;

std::string xbConnectionString;
std::string xbPublicKeyFile;
std::string xbPrivateKeyFile;
std::string xbKeyPassword;
std::string xbPort;
std::string xbNetworkInterface;

std::map<std::string, Manager*> xbManagers;

} // namespace xblab


int
main(int argc, char** argv) {
    if (argc != 2){
        fprintf(stderr, "usage: %s <config filename>\n", argv[0]);
        return 1;
    }

    if (xblab::Server::getConfig(argv[1]) != XBGOOD){
        return 1;
    }

    int port = atoi(xblab::xbPort.c_str());

    xblab::loop = uv_default_loop();
    uv_tcp_t server;
    uv_tcp_init(xblab::loop, &server);

    struct sockaddr_in bind_addr = uv_ip4_addr(
        xblab::xbNetworkInterface.c_str(), port);
    uv_tcp_bind(&server, bind_addr);
    int r = uv_listen(
        (uv_stream_t*) &server, XBMAXCONCURRENT, xblab::on_connect);
    if (r) {
        fprintf(stderr, "Listen error %s\n",
            uv_err_name(uv_last_error(xblab::loop)));
        return 1;
    }
    char *procname = (strncmp(argv[0], "./", 2)) ? argv[0] : argv[0] + 2;
    printf("%s listening on port %d\n", procname, port);
    return uv_run(xblab::loop, UV_RUN_DEFAULT);
}
