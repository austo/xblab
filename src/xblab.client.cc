#include <iostream>
#include <botan/botan.h>
#include "xblab.client.h"
#include "participant.h"

// Binding wrapper for Node.js plugin
namespace xblab {

#define LOG(msg) printf("%s\n",msg);
#define THROW(msg) return ThrowException(Exception::Error(String::New(msg)));

using namespace v8;
using namespace node;

// V8 entry point
void Xblab::InitAll(Handle<Object> module) {
  try {
    // Start up crypto - happens only once per addon load
    Botan::LibraryInitializer init("thread_safe=true");
  }
  catch(std::exception& e) {
    std::cerr << e.what() << "\n";
  }

  Participant::Init();
  module->Set(String::NewSymbol("createParticipant"),
      FunctionTemplate::New(CreateParticipant)->GetFunction());
   
}

Handle<Value> Xblab::CreateParticipant(const Arguments& args) {
    HandleScope scope;
    return scope.Close(Participant::NewInstance(args));
}

} //namespace xblab


/*
  Intenal class access from JS is difficult due to name mangling in C++
  Use extern C here to initalize module handle.
  Static class participants are especially problematic, look for a workaround
*/
extern "C" {
  static void init(v8::Handle<v8::Object> module) {    
    xblab::Xblab::InitAll(module);
  }  
  NODE_MODULE(xblab, init);
}
