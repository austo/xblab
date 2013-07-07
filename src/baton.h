#ifndef BATON_H
#define BATON_H

#include <string>


namespace xblab {


#ifndef XBLAB_NATIVE

#include <node.h>


struct DataBaton {
    DataBaton(v8::Local<v8::Function> cb){
        uvWork.data = this;
        callback = v8::Persistent<v8::Function>::New(cb);
    }
    ~DataBaton(){
        callback.Dispose();
    }
    uv_work_t uvWork;
    std::string xBuffer;
    std::string nonce;
    std::string url;
    std::string err;
    void *auxData;
    v8::Persistent<v8::Function> callback;
};

#else

#include <uv.h>
#include "member.h"

struct DataBaton {
    DataBaton(){
        uvWork.data = this;
        uvClient.data = this;
        uvWrite.data = this;
    }
    ~DataBaton(){}

    uv_work_t uvWork;
    uv_tcp_t uvClient;
    uv_stream_t *uvServer;
    uv_buf_t uvBuf;
    uv_write_t uvWrite;

    std::string xBuffer;
    std::string nonce;
    std::string url;
    std::string err;

    Member *member;
    void *auxData;    
};


#endif


} // namespace xblab

#endif
