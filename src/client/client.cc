#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <assert.h>

#include <iostream>
#include <string>

#include "client/memberBaton.h"
#include "common/macros.h"
#include "common/common.h"
#include "client/client.h"
#include "client/binding/xbClient.h"

using namespace std;

namespace xblab {

extern "C" {
  void on_connect(uv_connect_t *req, int status) {
    Client::onConnect(req, status);
  }

  void
  on_close(uv_handle_t* handle) {
    MemberBaton *baton =
      reinterpret_cast<MemberBaton *>(handle->data);
    baton->wrapper->emitEndConnection();
  }
}

// global uv buffer allocator
uv_buf_t
allocBuf(uv_handle_t* handle, size_t suggested_size) {
  return uv_buf_init((char *)malloc(suggested_size), suggested_size);
}

// xbClient.cc
extern uv_loop_t *loop;

extern string xbPublicKeyFile;
extern string xbServerAddress;
extern string xbServerPort;


void
Client::onRead(uv_stream_t* server, ssize_t nread, uv_buf_t buf) {

  if (nread == -1) {
    if (uv_last_error(loop).code != UV_EOF) {
      fprintf(stderr, "Read error %s\n", 
        uv_err_name(uv_last_error(loop)));
    }
  }
  
  MemberBaton *baton = reinterpret_cast<MemberBaton *>(server->data);

  if (nread == EOF){
    uv_close((uv_handle_t*)baton->uvServer, on_close);
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


void
Client::onReadWork(uv_work_t *r) {
  MemberBaton *baton = reinterpret_cast<MemberBaton *>(r->data);
  
  baton->needsJsCallback = false;
  baton->digestBroadcast();
}


void
Client::afterOnRead(uv_work_t *r) {
  MemberBaton *baton = reinterpret_cast<MemberBaton *>(r->data);
  if (baton->needsJsCallback){
    // call stored XbClient member function
    baton->needsJsCallback = false;
    baton->jsCallbackFactory(baton->wrapper);
  }
  if (baton->needsUvWrite) {
    writeBatonBuffer(baton);
  }
  if (baton->err != "") {
    cout << "afterOnRead error: " << baton->err;
  }
}


void
Client::onSendCredential(MemberBaton *baton) {
  int status = uv_queue_work(
    loop,
    &baton->uvWork,
    sendCredentialWork,
    (uv_after_work_cb)afterSendCredential);
  assert(status == XBGOOD);   
}


void
Client::sendCredentialWork(uv_work_t *r) {
  MemberBaton *baton = reinterpret_cast<MemberBaton *>(r->data);
  baton->packageCredential();
}


void
Client::afterSendCredential(uv_work_t *r) {
  MemberBaton *baton = reinterpret_cast<MemberBaton *>(r->data);
  baton->uvWriteCb = onWrite;

  uv_write(
    &baton->uvWrite,
    (uv_stream_t*)&baton->uvClient,
    &baton->uvBuf,
    1,
    baton->uvWriteCb
  );
}


void
Client::onWrite(uv_write_t *req, int status) {
  if (status == -1) {
    fprintf(stderr, "Write error %s\n",
      uv_err_name(uv_last_error(loop)));
    return;
  }  
}


void
Client::writeBatonBuffer(MemberBaton *baton) {
#ifdef TRACE
  cout << "inside writeBatonBuffer\n";
#endif

  uv_write(
    &baton->uvWrite,
    (uv_stream_t*)&baton->uvClient,
    &baton->uvBuf,
    1,
    baton->uvWriteCb);
  baton->needsUvWrite = false;
}


void
Client::onConnect(uv_connect_t *req, int status) {
  if (status == -1) {
    fprintf(stderr, "Error connecting to xblab server: %s\n",
      uv_err_name(uv_last_error(loop)));
    return;
  }

  MemberBaton *baton = reinterpret_cast<MemberBaton *>(req->data);
  baton->uvServer = req->handle;
  baton->uvReadCb = onRead;

  uv_read_start((uv_stream_t*) baton->uvServer, allocBuf, baton->uvReadCb);
}

} // namespace xblab
