#ifndef SERVER_H
#define SERVER_H

#include <map>
#include <uv.h>
#include <node.h>


namespace xblab {

class Xblab : public node::ObjectWrap {

// TODO: make xblab::Server an class instance class

public:
  // Creates the V8 objects & attaches them to the module (target)
  static void InitAll(v8::Handle<v8::Object>);
  static v8::Handle<v8::Value> SetConfig(const v8::Arguments& args);
  static v8::Handle<v8::Value> CreateManager(const v8::Arguments& args);
  static v8::Handle<v8::Value> OnConnect(const v8::Arguments& args);
  static v8::Handle<v8::Value> DigestBuf(const v8::Arguments& args);

  // static v8::Handle<v8::Value> Method(const v8::Arguments& args);
  static void OnConnectWork(uv_work_t *r);
  static void AfterOnConnect(uv_work_t *r);

  static void DigestBufWork(uv_work_t *r);
  static void AfterDigestBuf(uv_work_t *r);

  
  // TODO: change to pointers
  std::map<std::string, v8::Handle<v8::Object> > Managers;
  std::map<std::string, void*> mptrs;

private:
  Xblab();
  ~Xblab(){};
  static v8::Persistent<v8::Object> pHandle_;
};
} 
#endif
