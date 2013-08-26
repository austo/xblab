#ifndef SERVER_H
#define SERVER_H

#include <uv.h>

namespace xblab {

class Manager; // fwd decls
class MemberBaton;

class Server {
private:

  static void
  echoWrite(uv_write_t *req, int status);

  static void
  onWrite(uv_write_t *req, int status);

  static void
  fatal(const char *what);

  static void
  onConnectWork(uv_work_t *r);

  static void
  afterOnConnect (uv_work_t *r);

  static void
  onReadWork(uv_work_t *r);

  static void
  afterOnRead (uv_work_t *r);  


public:
  
  static int
  getConfig(char* filename);

  static void
  onConnect(uv_stream_t *server, int status);

  static void
  onRead(uv_stream_t *client, ssize_t nread, uv_buf_t buf);
};

// C linkage for libuv callback
extern "C" {

  void
  on_connect(uv_stream_t *server, int status);

  void
  on_close(uv_handle_t* handle);

  void
  on_write(uv_write_t *req, int status);
}

} // namespace xblab

#endif