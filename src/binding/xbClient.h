#ifndef XBCLIENT_H
#define XBCLIENT_H

#include <string>
#include <node.h>

namespace xblab {

class XbClient : public node::ObjectWrap {

  public:

    //static void Init();
    static v8::Handle<v8::Value> New(const v8::Arguments& args);
    static v8::Handle<v8::Value> GetHandle(v8::Local<v8::String>, const v8::AccessorInfo&);
    static void SetHandle(v8::Local<v8::String>, v8::Local<v8::Value>, const v8::AccessorInfo&);
    static v8::Handle<v8::Value> DigestBuffer(const v8::Arguments& args);
    static v8::Handle<v8::Value> SendCred(const v8::Arguments& args);

    friend class Util;     


  private:
    XbClient(std::string group = "none");
    ~XbClient(){};        

    std::string username_;
    std::string password_;
    std::string handle_;
    std::string group_;
    std::string pub_key_;
    std::string priv_key_;
    // std::string initial_server_key_;
    std::string session_server_key_;
    std::string round_nonce_;   //may need to rethink this
    std::string return_nonce_;
    int round_modulus_;    
};

} //namespace xblab

#endif