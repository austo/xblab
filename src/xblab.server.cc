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
        string cbuf = Util::get_need_cred_buf();

        std::vector<unsigned char> bytes(cbuf.begin(), cbuf.end());
        unsigned char *c = &bytes[0];

        size_t len = bytes.size();

        cout << "len: " << len << endl;
        // This is Buffer that actually makes heap-allocated raw binary available
        // to userland code.
        node::Buffer *slowBuffer = node::Buffer::New(bytes.size());

        // Buffer:Data gives us a yummy void* pointer to play with to our hearts
        // content.
        memcpy(node::Buffer::Data(slowBuffer), c, bytes.size());

        // Now we need to create the JS version of the Buffer I was telling you about.
        // To do that we need to actually pull it from the execution context.
        // First step is to get a handle to the global object.
        v8::Local<v8::Object> globalObj = v8::Context::GetCurrent()->Global();

        // Now we need to grab the Buffer constructor function.
        v8::Local<v8::Function> bufferConstructor = v8::Local<v8::Function>::Cast(globalObj->Get(v8::String::New("Buffer")));

        // Great. We can use this constructor function to allocate new Buffers.
        // Let's do that now. First we need to provide the correct arguments.
        // First argument is the JS object Handle for the SlowBuffer.
        // Second arg is the length of the SlowBuffer.
        // Third arg is the offset in the SlowBuffer we want the .. "Fast"Buffer to start at.
        v8::Handle<v8::Value> constructorArgs[3] = { slowBuffer->handle_, v8::Integer::New(bytes.size()), v8::Integer::New(0) };

        // Now we have our constructor, and our constructor args. Let's create the 
        // damn Buffer already!
        //v8::Local<v8::Object> actualBuffer = bufferConstructor->NewInstance(3, constructorArgs);



        argv[0] = Local<Value>::New(Undefined());
        argv[1] = bufferConstructor->NewInstance(3, constructorArgs);

        //Debug
        //cout << Util::parse_buf(cbuf);
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
