#ifndef PARTICIPANT_BATON_H
#define PARTICIPANT_BATON_H

#include <uv.h>
#include <v8.h>

#include "baton.h"
#include "native/participant.h"


namespace xblab {

class XbClient; // forward declaration

// pointer to member callback
typedef v8::Handle<v8::Value> (*JsCallbackFactory)(XbClient*);

class ParticipantBaton : public DataBaton {
public:
  explicit ParticipantBaton(uv_connect_t *req);
  explicit ParticipantBaton(uv_connect_cb cb);
  ParticipantBaton();
  ~ParticipantBaton(){}

  void stringifyBuffer();
  void createCredential();
  bool hasKeys();
  void getKeys();
  void digestBroadcast();

  Participant participant;

  uv_connect_t *uvConnect;
  uv_stream_t *uvServer;
  uv_buf_t uvBuf;
  uv_write_t uvWrite;
  uv_write_cb uvWriteCb; // where we go from here
  uv_read_cb uvReadCb;
  uv_connect_cb uvConnectCb;

  XbClient *wrapper;
  bool needsJsCallback;
  JsCallbackFactory jsCallbackFactory;


  // TODO: add uv_work_cb and uv_after_work_cb?
   
  
private:
  std::string privateKey_;

  // uv_write_cb's:


};

} // namespace xblab

#endif