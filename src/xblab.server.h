#ifndef XBLAB_H
#define XBLAB_H

#include <map>
#include <node.h>

#include "user.h"

namespace xblab {

class Xblab : public node::ObjectWrap {

// TODO: make xblab::Server an class instance class

public:
  // Creates the V8 objects & attaches them to the module (target)
  static void InitAll(v8::Handle<v8::Object>);
  static v8::Handle<v8::Value> SetConfig(const v8::Arguments& args);
  static v8::Handle<v8::Value> CreateManager(const v8::Arguments& args);
  static v8::Handle<v8::Value> OnConnection(const v8::Arguments& args);
  static v8::Handle<v8::Value> DigestBuffer(const v8::Arguments& args);

  std::map<int, User> CurrentUsers;
  std::map<std::string, v8::Handle<v8::Value> > Managers;



private:
  Xblab();
  ~Xblab(){};
  static v8::Persistent<v8::Object> pHandle_;
};
} 
#endif
