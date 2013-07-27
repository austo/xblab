#ifndef XBCLIENT_H
#define XBCLIENT_H

#include <string>
#include <node.h>

#include "client/memberBaton.h"

namespace xblab {

class XbClient : public node::ObjectWrap {

public:

  // TODO: handle "end" event - server disconnect

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
  groupEntryFactory(XbClient*);

  static v8::Handle<v8::Value>
  endConnectionFactory(XbClient*);

  // static v8::Handle<v8::Value>
  // errorFactory(XbClient*);

  static v8::Handle<v8::Value>
  SendCredential(const v8::Arguments& args);

  static v8::Handle<v8::Value>
  Transmit(const v8::Arguments& args);

  static v8::Handle<v8::Value>
  Connect(const v8::Arguments& args);

  MemberBaton *baton;

  v8::Handle<v8::Value>
  emitEndConnection(); 

private:
  XbClient(std::string group = "none");
  ~XbClient();

  bool
  hasMember();

  void
  initializeBaton(/*uv_connect_cb cb*/);

  v8::Handle<v8::Value>
  emitRequestCredential();

  v8::Handle<v8::Value>
  emitGroupEntry();

  // v8::Handle<v8::Value>
  // emitError();  

  std::string group_;
  static v8::Persistent<v8::Object> pHandle_;

};

} //namespace xblab

#endif