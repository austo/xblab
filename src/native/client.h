#ifndef CLIENT_H
#define CLIENT_H

#include <uv.h>

namespace xblab {

class Client {
private:

  static void afterOnRead (uv_work_t *r);
  static void onReadWork(uv_work_t *r);
  static void sendCredentialWork(uv_work_t *r);
  static void afterSendCredential(uv_work_t *r);

public:
  static void onConnect(uv_connect_t *req, int status);
  static void onRead(uv_stream_t *client, ssize_t nread, uv_buf_t buf);
  static void writeSendCredential(uv_write_t *req, int status);
  static void onSendCredential(ParticipantBaton *baton);

};

extern "C" {
  void on_connect(uv_connect_t *req, int status);
}

} // namespace xblab

#endif