#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <assert.h>

#include <iostream>
#include <string>

#include <yajl/yajl_tree.h>

#include "crypto.h"
#include "util.h"
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

/* C linkage for libuv
 * (we're kind of stubborn about making things class members)
 */
extern "C" {
    void on_connect(uv_stream_t *server, int status){
        xblab::Server::onConnect(server, status);
    }
}

/* public */
void
xblab::Server::onConnect(uv_stream_t *server, int status) {
    if (status == -1) {
        // error!
        return;
    }

    DataBaton *baton = new DataBaton();
    baton->server = server;
    status = uv_queue_work(
        loop,
        &baton->request,
        onConnectWork,
        (uv_after_work_cb)afterOnConnect);
    assert(status == XBGOOD);    
}

void
Server::onConnectWork(uv_work_t *r){
    DataBaton *baton = reinterpret_cast<DataBaton *>(r->data);

    string nonce;
    // Get "NEEDCRED" buffer
    string buf = Util::needCredBuf(nonce);
    baton->nonce = nonce;
    baton->buf = buf;
    baton->uvBuf.base = &baton->buf[0];
    baton->uvBuf.len = baton->buf.size();
}

void
Server::afterOnConnect (uv_work_t *r) {
    DataBaton *baton = reinterpret_cast<DataBaton *>(r->data);
    uv_tcp_init(loop, &baton->client);

    if (uv_accept(baton->server, (uv_stream_t*) &baton->client) == XBGOOD) {
        uv_write(&baton->writeRequest,
            (uv_stream_t*)&baton->client, &baton->uvBuf, 1, writeNeedCred);        
    }
    else {
        uv_close((uv_handle_t*) &baton->client, NULL);
    }
}


/*
 * NOTE: yajl doesn't seem to play nicely with protobuf,
 * although protobuf-lite appears to be okay.
 */
int
Server::getConfig(char* filename) {

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


/* private */
// allocates unattached uv_buf_t's
uv_buf_t
Server::allocBuf(uv_handle_t *handle, size_t suggested_size) {
    return uv_buf_init((char *)malloc(suggested_size), suggested_size);
}


void
Server::writeNeedCred(uv_write_t *req, int status) {
    if (status == -1) {
        fprintf(stderr, "Write error %s\n",
            uv_err_name(uv_last_error(loop)));
    }
    else {
        DataBaton *baton = reinterpret_cast<DataBaton *>(req->data);

        uv_read_start((uv_stream_t*) &baton->client, allocBuf, echoRead);
    }    
}


void
Server::echoRead(uv_stream_t *client, ssize_t nread, uv_buf_t buf) {
    if (nread == -1) {
        if (uv_last_error(loop).code != UV_EOF){
            fprintf(stderr, "Read error %s\n", 
                uv_err_name(uv_last_error(loop)));
        }
        uv_close((uv_handle_t*) client, NULL);
        return;
    }

    uv_write_t *req = (uv_write_t *) malloc(sizeof(uv_write_t));
    req->data = (void*) buf.base;
    buf.len = nread;
    uv_write(req, client, &buf, 1, echoWrite);

    // TODO: hook up to next step in protocol
    // DataBaton *baton = reinterpret_cast<DataBaton *>(client->data);

    // // test: generate nonce
    // // baton->nonce = Crypto::generateNonce();
    // string credbuf = Util::needCredBuf(baton->nonce);

    // uv_write_t *req = (uv_write_t *) malloc(sizeof(uv_write_t));

    // req->data = &credbuf[0]; // &baton->nonce[0];
    // buf.base =  &credbuf[0]; // &baton->nonce[0];
    // buf.len = credbuf.size(); // baton->nonce.size();
    // uv_write(req, client, &buf, 1, echoWrite);
}

void Server::echoWrite(uv_write_t *req, int status) {
    if (status == -1) {
        fprintf(stderr, "Write error %s\n", uv_err_name(uv_last_error(loop)));
    }
    char *base = (char*) req->data;
    free(base);
    free(req);
}


void
Server::fatal(const char *what) {
    uv_err_t err = uv_last_error(uv_default_loop());
    fprintf(stderr, "%s: %s\n", what, uv_strerror(err));
    exit(1);
}

} // namespace xblab
