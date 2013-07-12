#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <assert.h>

#include <iostream>
#include <string>

#include "native/participantBaton.h"
#include "macros.h"
#include "native/client.h"
#include "binding/xbClient.h"


namespace xblab {

// global uv buffer allocator
uv_buf_t
allocBuf(uv_handle_t *handle, ssize_t suggested_size) {
  return uv_buf_init((char *)malloc(suggested_size), suggested_size);
}

// xbClient.cc
extern uv_loop_t *loop;

extern string xbPublicKeyFile;
extern string xbServerAddress;
extern string xbServerPort;

/* C linkage for uv_tcp_connect */
// extern "C" {
//   void on_connect(uv_connect_t *req, int status){
//     Client::onConnect(req, status);
//   }
// }

// void
// Client::echoRead(uv_stream_t *server, ssize_t nread, uv_buf_t buf) {
//   if (nread == -1) {
//     fprintf(stderr, "error echo_read");
//     return;
//   }

//   printf("result: %s\n", buf.base);
// }


void
Client::onRead(uv_stream_t* server, ssize_t nread, uv_buf_t buf) {

  if (nread == -1) {
    if (uv_last_error(loop).code != UV_EOF) {
      fprintf(stderr, "Read error %s\n", 
        uv_err_name(uv_last_error(loop)));
    }
  }

  ParticipantBaton *baton = reinterpret_cast<ParticipantBaton *>(server->data);
  baton->uvBuf = buf;
  baton->uvBuf.len = nread;

  int status = uv_queue_work(
    loop,
    &baton->uvWork,
    onReadWork,
    afterOnRead);
  assert(status == XBGOOD);   
}


void
Client::onReadWork(uv_work_t *r){
  ParticipantBaton *baton = reinterpret_cast<ParticipantBaton *>(r->data);
  if (!baton->hasKeys()) {
    baton->getKeys();
  }
  baton->digestBroadcast();

}


void
afterOnRead(uv_work_t *r){
  ParticipantBaton *baton = reinterpret_cast<ParticipantBaton *>(r->data);
  if (baton->wrapperHasCallback){
    // call stored XbClient member function
    baton->wrapper->baton->jsCallback();
  }
}


// void
// Client::onWriteEnd(uv_write_t *req, int status) {
//   if (status == -1) {
//     fprintf(stderr, "error on_write_end");
//     return;
//   }

//   uv_read_start(req->handle, allocBuf, echoRead);
// }


void
Client::onConnect(uv_connect_t *req, int status) {
  if (status == -1) {
    fprintf(stderr, "Error connecting to xblab server: %s\n",
      uv_err_name(uv_last_error(loop)));
    return;
  }

  ParticipantBaton *baton = reinterpret_cast<ParticipantBaton *>(req->data);
  baton->uvServer = req->handle;
  baton->uvReadCb = onRead;

  uv_read_start((uv_stream_t*) baton->uvServer, allocBuf, baton->uvReadCb);  
}


} // namespace xblab
