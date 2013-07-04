#ifndef BATON_H
#define BATON_H

#include <string>


namespace xblab {


#ifndef XBLAB_NATIVE

#include <node.h>


struct DataBaton {
    DataBaton(v8::Local<v8::Function> cb){
        request.data = this;
        callback = v8::Persistent<v8::Function>::New(cb);
    }
    ~DataBaton(){
        callback.Dispose();
    }
    uv_work_t request;
    std::string buf;
    std::string nonce;
    std::string url;
    std::string err;
    void *auxData;
    v8::Persistent<v8::Function> callback;
};

#else

#include <uv.h>

struct DataBaton {
    DataBaton(){
        request.data = this;
        client.data = this;
    }
    ~DataBaton(){}
    uv_work_t request;
    std::string buf;
    std::string nonce;
    std::string url;
    std::string err;
    void *auxData;
    uv_tcp_t client;
};


#endif


} // namespace xblab

#endif
