#ifndef XBCLIENT_H
#define XBCLIENT_H

#include <string>
#include <node.h>

#include "native/participantBaton.h"

namespace xblab {

class XbClient : public node::ObjectWrap {

public:

  static v8::Handle<v8::Value>
  New(const v8::Arguments& args);

  static v8::Handle<v8::Value>
  GetHandle(v8::Local<v8::String>, const v8::AccessorInfo&);

  static void
  SetHandle(v8::Local<v8::String>,
    v8::Local<v8::Value>, const v8::AccessorInfo&);

  static v8::Handle<v8::Value>
  DigestBuffer(const v8::Arguments& args);

  static v8::Handle<v8::Value>
  requestCredentialFactory(XbClient*);

  static v8::Handle<v8::Value>
  SendCredential(const v8::Arguments& args);

  static v8::Handle<v8::Value>
  Connect(const v8::Arguments& args);
  

private:
  XbClient(std::string group = "none");
  ~XbClient();

  bool
  hasParticipant();

  void
  initializeBaton(/*uv_connect_cb cb*/);

  v8::Handle<v8::Value>
  requestCredential();

  std::string group_;
  ParticipantBaton *baton_;
  static v8::Persistent<v8::Object> pHandle_;

};

} //namespace xblab

#endif