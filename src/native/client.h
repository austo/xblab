#ifndef CLIENT_H
#define CLIENT_H

#include <uv.h>

namespace xblab {

class Client {
private:
  static void echoWrite(uv_write_t *req, int status);
  static void echoRead(uv_stream_t *server, ssize_t nread, uv_buf_t buf);

  static void fatal(const char *what);
  static void onConnectWork(uv_work_t *r);
  static void afterOnConnect (uv_work_t *r);
  static void onReadWork(uv_work_t *r);
  static void afterOnRead (uv_work_t *r);
  static void onWriteEnd(uv_write_t *req, int status);


public:
  static void onConnect(uv_connect_t *req, int status);
  static void readBuf(uv_stream_t *client, ssize_t nread, uv_buf_t buf);

  // C linkage for libuv callback
extern "C" {
  void on_connect(uv_connect_t *req, int status);
}
  
};

} // namespace xblab

#endif