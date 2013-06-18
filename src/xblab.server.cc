#include <node_buffer.h>
#include <iostream>
#include <vector>
#include <botan/botan.h>
#include "macros.h"
#include "xblab.server.h"
#include "manager.h"
#include "util.h"


using namespace v8;
using namespace node;
using namespace std;


namespace xblab {

/*
    C-style global configuration items
    May be used from other classes but need to be defined extern
    TODO: is there a cleaner way to do this?
*/
v8::Persistent<v8::String> connstring;
v8::Persistent<v8::String> pub_key_filename;
v8::Persistent<v8::String> priv_key_filename;
v8::Persistent<v8::String> key_passphrase;


// V8 entry point
void Xblab::InitAll(Handle<Object> module) {
    try {
        // Start up crypto - happens only once per addon load
        Botan::LibraryInitializer init("thread_safe=true");
    }
    catch(std::exception& e) {
        std::cerr << e.what() << "\n";
    }

    Manager::Init();
    module->Set(String::NewSymbol("createManager"),
        FunctionTemplate::New(CreateManager)->GetFunction());

    module->Set(String::NewSymbol("config"),
        FunctionTemplate::New(SetConfig)->GetFunction());

    module->Set(String::NewSymbol("getConnectionBuffer"),
        FunctionTemplate::New(GetConnectionBuffer)->GetFunction());  
}

Handle<Value> Xblab::CreateManager(const Arguments& args) {
    HandleScope scope;
    String::Utf8Value s(connstring->ToString());
    //cout << "v8 -> connection string: " << *s << endl << "privkfile: " << *(String::Utf8Value(priv_key_filename)) << endl;
    return scope.Close(Manager::NewInstance(args));
}

Handle<Value> Xblab::SetConfig(const Arguments& args) {

    HandleScope scope;

    if (!args[0]->IsObject()) {
        THROW("xblab.config requires object argument");
    }

    Handle<Object> cfg = Handle<Object>::Cast(args[0]);
    Handle<Value> constr = cfg->Get(String::New("connstring"));
    Handle<Value> pubkfile = cfg->Get(String::New("pubKeyFile"));
    Handle<Value> privkfile = cfg->Get(String::New("privKeyFile"));
    Handle<Value> keypw = cfg->Get(String::New("keyPassphrase"));


    connstring = NODE_PSYMBOL(*(String::Utf8Value(constr->ToString())));
    pub_key_filename = NODE_PSYMBOL(*(String::Utf8Value(pubkfile->ToString())));
    priv_key_filename = NODE_PSYMBOL(*(String::Utf8Value(privkfile->ToString())));
    key_passphrase = NODE_PSYMBOL(*(String::Utf8Value(keypw->ToString())));

    return scope.Close(Undefined());
}

Handle<Value> Xblab::GetConnectionBuffer(const Arguments& args) {

    HandleScope scope;
    if (!args[0]->IsFunction()){
        THROW("xblab.getConnectionBuffer requires callback argument");
    }

    Local<Function> cb = Local<Function>::Cast(args[0]);
    const unsigned argc = 2;
    Local<Value> argv[argc];

    try{

        // TODO: refactor into separate method & investigate Native C++ method
        string cbuf = Util::get_need_cred_buf(); // serialized "NEEDCRED buffer (binary data)"        
        /*
            You could also use cbuf.data() for this next line,
            but C++11 strings are guaranteed to be
            allocated contiguously.
        */
        const char *c = &cbuf[0]; 
        size_t len = cbuf.size();
        const unsigned buf_argc = 3;

        Buffer *slowBuffer = Buffer::New(len);        
        memcpy(Buffer::Data(slowBuffer), c, len); // Buffer::Data = (void *)
        
        // Create JS buffer from node execution context
        Local<Function> buf_ctor = // Get Buffer constructor
            Local<Function>::Cast(Context::GetCurrent()->Global()->Get(String::New("Buffer")));
       
        Handle<Value> buf_argv[buf_argc] = { 
                slowBuffer->handle_, // JS SlowBuffer handle
                Integer::New(len), // SlowBuffer length
                Integer::New(0) }; // Offset where "FastBuffer" should start
            

        argv[0] = Local<Value>::New(Undefined());
        argv[1] = buf_ctor->NewInstance(buf_argc, buf_argv);

        // Debug
        // cout << Util::parse_buf(cbuf);
    }
    catch (util_exception& e){
        argv[0] = Local<Value>::New(String::New(e.what()));
        argv[1] = Local<Value>::New(Undefined());
    }
    cb->Call(Context::GetCurrent()->Global(), argc, argv);
    return scope.Close(Undefined());
}


/*
  Intenal class access from JS is difficult due to name mangling in C++
  Use extern C here to initalize module handle.
  Static class members are especially problematic, look for a workaround
*/

} //namespace xblab


extern "C" {
  static void init(v8::Handle<v8::Object> module) {    
    xblab::Xblab::InitAll(module);
  }  
  NODE_MODULE(xblab, init);
}
