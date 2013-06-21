#include <iostream>
#include <sstream>
#include <node_buffer.h>
#include <botan/botan.h>
#include "util.h"
#include "macros.h"
#include "xblab.client.h"
#include "participant.h"


namespace xblab {


using namespace v8;
using namespace node;
using namespace std;


v8::Persistent<v8::String> pub_key_filename;
v8::Persistent<v8::Function> nodeBufCtor;

// V8 entry point
void Xblab::InitAll(Handle<Object> module) {
    try {
        // Start up crypto - happens only once per module load
        Botan::LibraryInitializer init("thread_safe=true");
    }
    catch(std::exception& e) {
        std::cerr << e.what() << "\n";
    }

    nodeBufCtor = XB_NODE_BUFFER_CTOR;
    
    Participant::Init();
    module->Set(String::NewSymbol("createParticipant"),
        FunctionTemplate::New(CreateParticipant)->GetFunction());

    module->Set(String::NewSymbol("config"),
        FunctionTemplate::New(SetConfig)->GetFunction());

    module->Set(String::NewSymbol("parseConnectionBuffer"),
        FunctionTemplate::New(ParseConnectionBuffer)->GetFunction());
     
}

Handle<Value> Xblab::CreateParticipant(const Arguments& args) {
    HandleScope scope;
    return scope.Close(Participant::NewInstance(args));
}


// TODO: error callback
Handle<Value> Xblab::SetConfig(const Arguments& args) {
    HandleScope scope;

    if (!args[0]->IsObject()) {
            THROW("xblab.config requires object argument");
    }

    Handle<Object> cfg = Handle<Object>::Cast(args[0]);
    Handle<Value> pubkfile = cfg->Get(String::New("pubKeyFile"));

    pub_key_filename = NODE_PSYMBOL(*(String::Utf8Value(pubkfile)));
    cout << *(String::Utf8Value(pub_key_filename)) << endl;

    return scope.Close(Undefined());
}

// This method should take in the connection buffer received by the server,
// then build the appropriate response, whereupon the JS callback
// (most likely a call to socket.write) is executed.

// TODO: Use node::MakeCallback?
// TODO: Parsing method should take a generic buffer and emit the appropriate
// response - then use EventEmitter
Handle<Value> Xblab::ParseConnectionBuffer(const Arguments& args) {
    HandleScope scope;

    // Parse binary data from node::Buffer
    char* bufData = Buffer::Data(args[0]->ToObject());
    int bufLen = Buffer::Length(args[0]->ToObject());

    if (!args[1]->IsFunction()){
        THROW("xblab.parseConnectionBuffer requires callback argument");
    }

    string buf(bufData, bufData + bufLen); // copy node::Buffer contents into string

    /*
        TODO: this next section should happen after we've formulated
        the appropriate response, which varies depending on the buffer
        we've just parsed. In any event, we're going to need to build
        the response buffer before returning to JS land.
    */
    Local<Function> cb = Local<Function>::Cast(args[1]);
    const unsigned argc = 2;
    Local<Value> argv[argc];

    try{
        argv[0] = Local<Value>::New(Undefined()); //Error
        argv[1] = Local<Value>::New(String::New(Util::parseBuf(buf).c_str()));      
    }
    catch (util_exception& e){
        argv[0] = Local<Value>::New(String::New(e.what()));
        argv[1] = Local<Value>::New(Undefined());
    }
    cb->Call(Context::GetCurrent()->Global(), argc, argv);
    return scope.Close(Undefined());
}

} //namespace xblab


// Use extern C here to initalize module handle w/o C++ name mangling.
extern "C" {
    static void init(v8::Handle<v8::Object> module) {    
        xblab::Xblab::InitAll(module);
    }  
    NODE_MODULE(xblab, init);
}
