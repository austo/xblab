#ifndef NODEUTIL_H
#define NODEUTIL_H

#include <node.h>
#include <string>


namespace xblab {


class NodeUtil {
public:

  static v8::Local<v8::Value> wrapBuf(const char *c, size_t len);

  static std::string v8ToString(v8::Local<v8::Value> value) {
    v8::String::Utf8Value utf8Value(value);
    return std::string(*utf8Value);
  }

private:
  NodeUtil(){};
  ~NodeUtil(){};

};


} //namespace xblab

#endif