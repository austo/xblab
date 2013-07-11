#ifndef PARTICIPANT_BATON_H
#define PARTICIPANT_BATON_H

#include "baton.h"

#include <node.h>

namespace xblab {

class ParticipantBaton : public DataBaton {
public:
  ParticipantBaton(v8::Local<v8::Function> cb){
    jsCallback = v8::Persistent<v8::Function>::New(cb);
  }
  ~ParticipantBaton(){
    jsCallback.Dispose();
  }    
  v8::Persistent<v8::Function> jsCallback;
};

} // namespace xblab

#endif