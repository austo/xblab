#ifndef XBLAB_H
#define XBLAB_H

#include <node.h>

namespace xblab {

class Xblab : public node::ObjectWrap {

public:
  // Creates the V8 objects & attaches them to the module (target)
  static void InitAll(v8::Handle<v8::Object>);
  static v8::Handle<v8::Value> SetConfig(const v8::Arguments& args);
  static v8::Handle<v8::Value> CreateManager(const v8::Arguments& args);
  static v8::Handle<v8::Value> OnConnection(const v8::Arguments& args);

private:
  Xblab(){};
  ~Xblab(){};
};
} 
#endif
