#ifndef SERVER_H
#define SERVER_H

#include <uv.h>

namespace xblab {

class Server {
private:
  static void echoWrite(uv_write_t *req, int status);
  static void fatal(const char *what);
  static void onConnectWork(uv_work_t *r);
  static void afterOnConnect (uv_work_t *r);
  static void onReadWork(uv_work_t *r);
  static void afterOnRead (uv_work_t *r);


public:
  static int getConfig(char* filename);
  static void onConnect(uv_stream_t *server, int status);
  static void readBuf(uv_stream_t *client, ssize_t nread, uv_buf_t buf);
};

// C linkage for libuv callback
extern "C" {
  void on_connect(uv_stream_t *server, int status);
  void on_close(uv_handle_t* handle);
}

} // namespace xblab

#endif