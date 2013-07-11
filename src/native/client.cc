#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <assert.h>

#include <iostream>
#include <string>

#include "participantBaton.h"
#include "macros.h"
#include "client.h"

namespace xblab {

// xbClient.cc
extern uv_loop_t *loop;
extern uv_buf_t allocBuf(uv_handle_t *handle, size_t suggested_size);

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
    fprintf(stderr, "Error connecting to xblab server.\n");
    return;
  }

  // init participant baton
  ParticipantBaton *baton = new ParticipantBaton(req); 


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
