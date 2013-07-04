#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <string>

#include <uv.h>

#include "crypto.h"

namespace xblab {


struct DataBaton {
    DataBaton(){
        request.data = this;
        client.data = this;
        write_req.data = this;
    }
    ~DataBaton(){}
    uv_tcp_t client;
    uv_work_t request;
    uv_write_t write_req;
    std::string buf;
    std::string nonce;
    std::string err;
    void *auxData;
};


uv_loop_t *loop;

std::string publicKeyFile = "rsapub.pem";
std::string privateKeyFile = "rsapriv.pem";
std::string keyPassword = "aardvark";


uv_buf_t alloc_buffer(uv_handle_t *handle, size_t suggested_size) {
    return uv_buf_init((char *) malloc(suggested_size), suggested_size);
}

void echo_write(uv_write_t *req, int status) {
    if (status == -1) {
        fprintf(stderr, "Write error %s\n", uv_err_name(uv_last_error(loop)));
    }

    DataBaton *baton = reinterpret_cast<DataBaton *>(req->data);
    char *base = (char*) baton->auxData;
    free(base);
    free(req);
}

void echo_read(uv_stream_t *client, ssize_t nread, uv_buf_t buf) {
    if (nread == -1) {
        if (uv_last_error(loop).code != UV_EOF){
            fprintf(stderr, "Read error %s\n", uv_err_name(uv_last_error(loop)));
        }
        uv_close((uv_handle_t*) client, NULL);
        return;
    }


    // uv_write_t *req = (uv_write_t *) malloc(sizeof(uv_write_t));
    DataBaton *baton = reinterpret_cast<DataBaton *>(client->data);

    // grab nonce
    baton->nonce = Crypto::generateNonce();

    baton->auxData = buf.base;
    buf.len = nread + sizeof(baton->nonce.data()); // might use + baton->nonce.size()
    uv_write(&baton->write_req, client, &buf, 1, echo_write);
}

void on_new_connection(uv_stream_t *server, int status) {
    if (status == -1) {
        // error!
        return;
    }

    DataBaton *baton = new DataBaton();
    // uv_tcp_t *client = (uv_tcp_t*) malloc(sizeof(uv_tcp_t));
    uv_tcp_init(loop, &baton->client);
    if (uv_accept(server, (uv_stream_t*) &baton->client) == 0) {
        uv_read_start((uv_stream_t*) &baton->client, alloc_buffer, echo_read);
    }
    else {
        uv_close((uv_handle_t*) &baton->client, NULL);
    }
}

} // namespace xblab


int main(int argc, char** argv) {
    if (argc != 2){
        fprintf(stderr, "usage: %s port\n", argv[0]);
        return 1;
    }
    int port = atoi(argv[1]);

    xblab::loop = uv_default_loop();
    uv_tcp_t server;
    uv_tcp_init(xblab::loop, &server);

    struct sockaddr_in bind_addr = uv_ip4_addr("127.0.0.1", port);
    uv_tcp_bind(&server, bind_addr);
    int r = uv_listen((uv_stream_t*) &server, 128, xblab::on_new_connection);
    if (r) {
        fprintf(stderr, "Listen error %s\n", uv_err_name(uv_last_error(xblab::loop)));
        return 1;
    }
    return uv_run(xblab::loop, UV_RUN_DEFAULT);
}

