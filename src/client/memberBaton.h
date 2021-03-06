#ifndef MEMBER_BATON_H
#define MEMBER_BATON_H

#include <uv.h>
#include <v8.h>

#include "common/baton.h"
#include "client/member.h"


namespace xblab {

class XbClient; // forward declaration

// Pointer to static ObjectWrap JS callback factory method 
typedef v8::Handle<v8::Value> (*JsCallbackFactory)(XbClient*);

class MemberBaton : public DataBaton {
public:
  explicit MemberBaton(uv_connect_t *req);
  explicit MemberBaton(uv_connect_cb cb);
  MemberBaton();
  ~MemberBaton();

  void createCredential();
  void digestBroadcast();
  void packageCredential();

  Member member;

  uv_connect_t uvConnect;  
  uv_connect_cb uvConnectCb;

  std::string roundMessage;
  bool hasRoundMessage;

  XbClient *wrapper;
  bool needsJsCallback;
  JsCallbackFactory jsCallbackFactory;

};

} // namespace xblab

#endif