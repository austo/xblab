#ifndef PARTICIPANT_BATON_H
#define PARTICIPANT_BATON_H

#include <uv.h>
#include <v8.h>

#include "common/baton.h"
#include "client/participant.h"


namespace xblab {

class XbClient; // forward declaration

// Pointer to static ObjectWrap JS callback factory method 
typedef v8::Handle<v8::Value> (*JsCallbackFactory)(XbClient*);

class ParticipantBaton : public DataBaton {
public:
  explicit ParticipantBaton(uv_connect_t *req);
  explicit ParticipantBaton(uv_connect_cb cb);
  ParticipantBaton();
  ~ParticipantBaton();

  void createCredential();
  bool hasKeys();
  void getKeys();
  void digestBroadcast();
  void packageCredential();

  Participant participant;

  uv_connect_t uvConnect;  
  uv_connect_cb uvConnectCb;

  XbClient *wrapper;
  bool needsJsCallback;
  JsCallbackFactory jsCallbackFactory;


  // TODO: add uv_work_cb and uv_after_work_cb?
  // uv_write_cb's:

};

} // namespace xblab

#endif