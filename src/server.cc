#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <iostream>
#include <string>

#include <yajl/yajl_tree.h>

#include "crypto.h"
#include "baton.h"
#include "macros.h"
#include "server.h"

using namespace std;

namespace xblab {


// xblab.cc
extern string xbConnectionString;
extern string xbPublicKeyFile;
extern string xbPrivateKeyFile;
extern string xbKeyPassword;
extern string xbPort;

extern uv_loop_t *loop;


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


extern "C" {
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
}

int
get_config(char* filename) {

    FILE *fd = fopen (filename, "r");
    if (fd == NULL) {
        fprintf(stderr, "invalid input: %s\n", filename);
        return 1;
    }

    fseek(fd, 0, SEEK_END);
    long fsize = ftell(fd);
    rewind(fd);

    char *cfgbuf = (char *)malloc(fsize + 1);
    if (cfgbuf == NULL) {
        fprintf(stderr, "memory error\n");
        return 1;
    }

    // NULL plug err and cfg buffers
    char errbuf[YAJL_ERR_BUF_SZ];
    cfgbuf[0] = errbuf[0] = 0;

    // Read whole config file
    size_t rd = fread((void *)cfgbuf, 1, fsize, fd);

    if (rd == 0 && !feof(fd)) {
        fprintf(stderr, "error reading config file\n");
        return 1;
    }    

    // Let's do some JSON parsing...
    yajl_val node = yajl_tree_parse(cfgbuf, errbuf, YAJL_ERR_BUF_SZ);

    if (node == NULL) {
        PRINT_YAJL_ERRBUF(errbuf);
        return 1;
    }

    // These need to be named exactly like their JSON counterparts
    GET_PROP(xbConnectionString) /* TODO: get yajl_status */
    GET_PROP(xbPublicKeyFile)
    GET_PROP(xbPrivateKeyFile)
    GET_PROP(xbKeyPassword)
    GET_PROP(xbPort);

    fclose(fd);
    free(cfgbuf);
    yajl_tree_free(node);

    return 0;
}

void
fatal(const char *what) {
    uv_err_t err = uv_last_error(uv_default_loop());
    fprintf(stderr, "%s: %s\n", what, uv_strerror(err));
    exit(1);
}

} // namespace xblab
