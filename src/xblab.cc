#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <string>

#include "macros.h"
#include "server.h"

namespace xblab {

uv_loop_t *loop;

std::string xbConnectionString;
std::string xbPublicKeyFile;
std::string xbPrivateKeyFile;
std::string xbKeyPassword;
std::string xbPort;

extern "C" {    

    int
    main(int argc, char** argv) {
        if (argc != 2){
            fprintf(stderr, "usage: %s <config filename>\n", argv[0]);
            return 1;
        }

        if (Server::get_config(argv[1]) != XBGOOD){
            return 1;
        }

        int port = atoi(xbPort.c_str());

        loop = uv_default_loop();
        uv_tcp_t server;
        uv_tcp_init(loop, &server);

        struct sockaddr_in bind_addr = uv_ip4_addr("127.0.0.1", port);
        uv_tcp_bind(&server, bind_addr);
        int r = uv_listen((uv_stream_t*) &server, 128, on_connection);
        if (r) {
            fprintf(stderr, "Listen error %s\n",
                uv_err_name(uv_last_error(loop)));
            return 1;
        }
        char *procname = (strncmp(argv[0], "./", 2)) ? argv[0] : argv[0] + 2;
        printf("%s listening on port %d\n", procname, port);
        return uv_run(loop, UV_RUN_DEFAULT);
    }
}

} // namespace xblab