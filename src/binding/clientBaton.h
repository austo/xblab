#ifndef CLIENT_BATON_H
#define CLIENT_BATON_H

#include "baton.h"

#include <node.h>

namespace xblab {

class ClientBaton : public DataBaton {
public:
    ClientBaton(v8::Local<v8::Function> cb){
        callback = v8::Persistent<v8::Function>::New(cb);
    }
    ~ClientBaton(){
        callback.Dispose();
    }    
    v8::Persistent<v8::Function> callback;
};

} // namespace xblab

#endif
