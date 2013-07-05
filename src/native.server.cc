#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <iostream>
#include <string>

#include <uv.h>

#include <yajl/yajl_tree.h>

#include "crypto.h"
#include "baton.h"
#include "macros.h"

namespace xblab {


// Defined extern in crypto.cc
std::string xbConnectionString;
std::string xbPublicKeyFile;
std::string xbPrivateKeyFile;
std::string xbKeyPassword;
char *xbPort;

uv_loop_t *loop;

// TODO: make configurable
static unsigned char cfgData[8192];


// TODO: integrate into DataBaton
uv_buf_t
alloc_buffer(uv_handle_t *handle, size_t suggested_size) {
    return uv_buf_init((char *)malloc(suggested_size), suggested_size);
}


// TODO: should really be called after_write
void
echo_write(uv_write_t *req, int status) {
    if (status == -1) {
        fprintf(stderr, "Write error %s\n",
            uv_err_name(uv_last_error(loop)));
    }

    // req is temporary for each write request
    free(req);
}


void
echo_read(uv_stream_t *client, ssize_t nread, uv_buf_t buf) {
    if (nread == -1) {
        if (uv_last_error(loop).code != UV_EOF){
            fprintf(stderr, "Read error %s\n", 
                uv_err_name(uv_last_error(loop)));
        }
        uv_close((uv_handle_t*) client, NULL);
        return;
    }

    DataBaton *baton = reinterpret_cast<DataBaton *>(client->data);

    // test: generate nonce
    baton->nonce = Crypto::generateNonce();
    uv_write_t *req = (uv_write_t *) malloc(sizeof(uv_write_t));

    req->data = &baton->nonce[0];
    buf.base = &baton->nonce[0];
    buf.len = baton->nonce.size();
    uv_write(req, client, &buf, 1, echo_write);
}


void
on_new_connection(uv_stream_t *server, int status) {
    if (status == -1) {
        // error!
        return;
    }

    DataBaton *baton = new DataBaton();
    uv_tcp_init(loop, &baton->client);

    // TODO: where does suggested size come from??
    if (uv_accept(server, (uv_stream_t*) &baton->client) == 0) {
        uv_read_start((uv_stream_t*) &baton->client, alloc_buffer, echo_read);
    }
    else {
        uv_close((uv_handle_t*) &baton->client, NULL);
    }
}


int
get_config_data(char* filename) {

    // Let's do some JSON parsing...
    FILE *fd;
    fd = fopen (filename, "r");
    if (fd == NULL) {
        fprintf(stderr, "invalid input: %s\n", filename);
        return 1;
    }

    size_t rd;
    yajl_val node;
    char errbuf[1024];

    // NULL plug buffers
    cfgData[0] = errbuf[0] = 0;

    // Read whole config file
    rd = fread((void *) cfgData, 1, sizeof(cfgData) - 1, fd);

    if (rd == 0 && !feof(stdin)) {
        fprintf(stderr, "error reading config file\n");
        return 1;
    }
    else if (rd >= sizeof(cfgData) - 1) {
        fprintf(stderr, "config file too big\n");
        return 1;
    }

    // Might as well parse it...
    node = yajl_tree_parse((const char *)cfgData,
        errbuf, sizeof(errbuf));

    if (node == NULL) {
        PRINT_YAJL_ERRBUF(errbuf);
        return 1;
    }    

    // These need to be named exactly like their JSON counterparts
    GET_PROP(xbConnectionString)
    GET_PROP(xbPublicKeyFile)
    GET_PROP(xbPrivateKeyFile)
    GET_PROP(xbKeyPassword)
    GET_PROP(xbPort);

    return 0;
}

} // namespace xblab


int
main(int argc, char** argv) {
    if (argc != 2){
        fprintf(stderr, "usage: %s <config filename>\n", argv[0]);
        return 1;
    }

    if (xblab::get_config_data(argv[1]) != XBGOOD){
        return 1;
    }

    int port = atoi(xblab::xbPort);

    xblab::loop = uv_default_loop();
    uv_tcp_t server;
    uv_tcp_init(xblab::loop, &server);

    struct sockaddr_in bind_addr = uv_ip4_addr("127.0.0.1", port);
    uv_tcp_bind(&server, bind_addr);
    int r = uv_listen((uv_stream_t*) &server, 128, xblab::on_new_connection);
    if (r) {
        fprintf(stderr, "Listen error %s\n",
            uv_err_name(uv_last_error(xblab::loop)));
        return 1;
    }
    return uv_run(xblab::loop, UV_RUN_DEFAULT);
}