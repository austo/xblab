#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <assert.h>

#include <iostream>
#include <string>

#include "native/participantBaton.h"
#include "macros.h"
#include "native/client.h"
#include "native/participantUtil.h"

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
extern "C" {
  void on_connect(uv_connect_t *req, int status){
    Client::onConnect(req, status);
  }
}

void
Client::echoRead(uv_stream_t *server, ssize_t nread, uv_buf_t buf) {
  if (nread == -1) {
    fprintf(stderr, "error echo_read");
    return;
  }

  printf("result: %s\n", buf.base);
}


// set as read callback only after connection (need credential)
void
Client::onFirstRead(uv_stream_t* server, ssize_t nread, uv_buf_t buf) {

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
    onFirstReadWork,
    afterFirstRead);
  assert(status == XBGOOD);   
}


void
Client::onFirstReadWork(uv_work_t *r){
  ParticipantBaton *baton = reinterpret_cast<ParticipantBaton *>(r->data);
  baton->stringifyBuffer();
  baton->err = "";
}


void
afterFirstRead(uv_work_t *r){

}


void
Client::onWriteEnd(uv_write_t *req, int status) {
  if (status == -1) {
    fprintf(stderr, "error on_write_end");
    return;
  }

  uv_read_start(req->handle, allocBuf, echoRead);
}


void
Client::onConnect(uv_connect_t *req, int status) {
  if (status == -1) {
    fprintf(stderr, "Error connecting to xblab server: %s\n",
      uv_err_name(uv_last_error(loop)));
    return;
  }

  ParticipantBaton *baton = reinterpret_cast<ParticipantBaton *>(req->data);
  baton->uvServer = req->handle;
  baton->uvReadCb = onFirstRead;

  // we expect a NEEDCRED message from the server
  uv_read_start((uv_stream_t*) baton->uvServer, allocBuf, baton->uvReadCb);

  // start uv_queue_work
  // ParticipantBaton *baton = reinterpret_cast<ParticipantBaton *>(req->data);
  // baton->setConnectionWork();
  // baton->uvServer = req->handle;
  // status = uv_queue_work(
  //   loop,
  //   &baton->uvWork,
  //   onConnectWork,
  //   (uv_after_work_cb)afterOnConnect);
  // assert(status == XBGOOD); 

  // char *message = "hello.txt";
  // int len = strlen(message);
  
  // char buffer[100];
  // uv_buf_t buf = uv_buf_init(buffer, sizeof(buffer));

  // buf.len = len;
  // buf.base = message;

  // uv_stream_t* tcp = req->handle;
  // uv_write_t write_req;

  // int buf_count = 1;
  // uv_write(&write_req, tcp, &buf, buf_count, onWriteEnd);
}


// int
// main(int argc, char** argv) {
//   int port = atoi(argv[1]);

//   loop = uv_default_loop();

//   uv_tcp_t client;
//   uv_tcp_init(loop, &client);

//   struct sockaddr_in req_addr = uv_ip4_addr("127.0.0.1", port);

//   uv_connect_t connect_req;
//   uv_tcp_connect(&connect_req, &client, req_addr, on_connect);

//   return uv_run(loop, UV_RUN_DEFAULT);
// }

} // namespace xblab
