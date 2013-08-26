#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <assert.h>

#include <iostream>
#include <string>
#include <map>

#include <yajl/yajl_tree.h>

#include "memberBaton.h"
#include "common/macros.h"
#include "common/common.h"
#include "server.h"
#include "manager.h"

using namespace std;


namespace xblab {

// xblab.cc
extern string xbConnectionString;
extern string xbPublicKeyFile;
extern string xbPrivateKeyFile;
extern string xbKeyPassword;
extern string xbPort;
extern string xbNetworkInterface;

extern map<string, Manager*> xbManagers;

extern uv_loop_t *loop;
extern uv_mutex_t xbMutex;
extern uv_buf_t allocBuf(uv_handle_t *handle, size_t suggested_size);

/* C linkage for libuv */
extern "C" {

  void
  on_connect(uv_stream_t *server, int status) {
    Server::onConnect(server, status);
  }

  void
  on_close(uv_handle_t* handle) {
    MemberBaton *baton = reinterpret_cast<MemberBaton *>(handle->data);

    // TODO: broadcast end chat
    baton->member->manager->endChat();
    delete baton;
  }

  void
  on_write(uv_write_t *req, int status) {
    // Will free everything if req was allocated
    // as (sizeof(uv_write_t) + buffer_length)
    free(req);
  }
}

/* public */

/* NOTE: yajl doesn't seem to play nicely with protobuf,
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

  // Do some JSON parsing...
  yajl_val node = yajl_tree_parse(cfgbuf, errbuf, YAJL_ERR_BUF_SZ);

  if (node == NULL) {
    PRINT_YAJL_ERRBUF(errbuf);
    return 1;
  }

  // These need to be named exactly like their JSON counterparts
  GET_PROP(xbConnectionString); /* TODO: get yajl_status */
  GET_PROP(xbPublicKeyFile);
  GET_PROP(xbPrivateKeyFile);
  GET_PROP(xbKeyPassword);
  GET_PROP(xbPort);
  GET_PROP(xbNetworkInterface);

  fclose(fd);
  free(cfgbuf);
  yajl_tree_free(node);

  return 0;
}


void
Server::onConnect(uv_stream_t *server, int status) {
  if (status == -1) {
    // TODO: handle meaningfully!
    return;
  }

  MemberBaton *baton = new MemberBaton();
  uv_tcp_init(loop, &baton->uvClient);

  baton->uvServer = server;
  status = uv_queue_work(
    loop,
    &baton->uvWork,
    onConnectWork,
    (uv_after_work_cb)afterOnConnect);
  assert(status == XBGOOD);    
}


void
Server::onRead(uv_stream_t *client, ssize_t nread, uv_buf_t buf) {
  MemberBaton *baton = reinterpret_cast<MemberBaton *>(client->data);

  if (nread == -1) {
    if (uv_last_error(loop).code != UV_EOF){
      fprintf(stderr, "Read error %s\n", 
        uv_err_name(uv_last_error(loop)));
    }
    uv_close((uv_handle_t*) client, on_close);
    return;
  }

  baton->uvBuf = buf;
  baton->uvBuf.len = nread;

  int status = uv_queue_work(
    loop,
    &baton->uvWork,
    onReadWork,
    (uv_after_work_cb)afterOnRead);
  assert(status == XBGOOD);    
}


/* private */

void
Server::onConnectWork(uv_work_t *r){
  MemberBaton *baton = reinterpret_cast<MemberBaton *>(r->data);
  baton->getNeedCredential();    
}


void
Server::afterOnConnect (uv_work_t *r) {
  MemberBaton *baton = reinterpret_cast<MemberBaton *>(r->data);
  uv_tcp_init(loop, &baton->uvClient);

  if (uv_accept(baton->uvServer,
    (uv_stream_t*) &baton->uvClient) == XBGOOD) {
    baton->uvReadCb = onRead;
    baton->uvWriteCb = onWrite;

    uv_write( // TODO: macroize
      &baton->uvWrite,
      (uv_stream_t*)&baton->uvClient,
      &baton->uvBuf,
      1,
      baton->uvWriteCb);

    // Start listening to client, now that they've authenticated
    uv_read_start((uv_stream_t*)&baton->uvClient, allocBuf, baton->uvReadCb);
  }
  else {
    uv_close((uv_handle_t*) &baton->uvClient, NULL);
  }
}


// Got some data from the client... unpack and forward
void
Server::onReadWork(uv_work_t *r){
  MemberBaton *baton = reinterpret_cast<MemberBaton *>(r->data);
  baton->stringifyBuffer();
  baton->err = "";
  baton->processTransmission(); // Util router accessed through baton
}


void
Server::afterOnRead(uv_work_t *r) {
  MemberBaton *baton = reinterpret_cast<MemberBaton *>(r->data);
  if (baton->err != ""){
    // TODO: send error and close
    uv_close((uv_handle_t*) &baton->uvClient, on_close);
    return;
  }

  if (baton->needsUvWrite) {
    uv_write(
      &baton->uvWrite,
      (uv_stream_t*)&baton->uvClient,
      &baton->uvBuf,
      1,
      baton->uvWriteCb);
  }

  respondAfterRead(baton->member->manager);
}


void
Server::respondAfterRead(Manager *mgr) {
  
  if (mgr->canStartChat()) {
    mgr->startChatIfNecessary(true);
    return;
  }
  if (mgr->canDeliverSchedules()) {
    mgr->deliverSchedulesIfNecessary(true);
    return;
  }
}


void
Server::echoWrite(uv_write_t *req, int status) {
  if (status == -1) {
    fprintf(stderr, "Write error %s\n", uv_err_name(uv_last_error(loop)));
  }
  char *base = (char*) req->data;
  free(base);
  free(req);
}


void
Server::onWrite(uv_write_t *req, int status) {
  if (status == -1) {
    fprintf(stderr, "Write error %s\n",
      uv_err_name(uv_last_error(loop)));
    return;
  }
}


void
Server::fatal(const char *what) {
  uv_err_t err = uv_last_error(uv_default_loop());
  fprintf(stderr, "%s: %s\n", what, uv_strerror(err));
  exit(1);
}

} // namespace xblab
