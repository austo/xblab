#ifndef XBCLIENT_H
#define XBCLIENT_H

#include <string>
#include "native/participantBaton.h"
#include <node.h>

namespace xblab {

class XbClient : public node::ObjectWrap {

public:
  static v8::Handle<v8::Value> New(const v8::Arguments& args);
  static v8::Handle<v8::Value> GetHandle(v8::Local<v8::String>, const v8::AccessorInfo&);
  static void SetHandle(v8::Local<v8::String>, v8::Local<v8::Value>, const v8::AccessorInfo&);
  static v8::Handle<v8::Value> DigestBuffer(const v8::Arguments& args);
  static v8::Handle<v8::Value> SendCredential(const v8::Arguments& args);
  static v8::Handle<v8::Value> Connect(const v8::Arguments& args);

private:
  XbClient(std::string group = "none");
  ~XbClient();

  bool hasParticipant();

  std::string group_;
  ParticipantBaton *baton_;
};

} //namespace xblab

#endif