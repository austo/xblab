#ifndef PARTICIPANT_BATON_H
#define PARTICIPANT_BATON_H

#include <node.h>

#include "baton.h"


namespace xblab {

class ParticipantBaton : public DataBaton {
public:
  explicit ParticipantBaton(uv_connect_t *req);
  explicit ParticipantBaton(uv_connect_cb cb);
  ~ParticipantBaton(){}

  void stringifyBuffer();
  void createCredential();

  std::string username;
  std::string password;
  std::string handle;
  std::string publicKey;
  std::string sessionServerKey;  
  int modulus;

  uv_connect_t uvConnect;
  uv_stream_t *uvServer;
  uv_buf_t uvBuf;
  uv_write_t uvWrite;
  uv_write_cb uvWriteCb; // where we go from here
  uv_read_cb uvReadCb;
  uv_connect_cb uvConnectCb;

  v8::Persistent<v8::Function> jsCallback;

  // TODO: add uv_work_cb and uv_after_work_cb?
   
  
private:
  std::string privateKey_;

  // uv_write_cb's:


};

} // namespace xblab

#endif