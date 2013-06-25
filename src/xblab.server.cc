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
    TODO: use object template?
*/
v8::Persistent<v8::String> connstring;
v8::Persistent<v8::String> pub_key_filename;
v8::Persistent<v8::String> priv_key_filename;
v8::Persistent<v8::String> key_passphrase;
v8::Persistent<v8::Function> nodeBufCtor;


// V8 entry point
// Repeat declaration of static member(s) to make visible to extern C
v8::Persistent<v8::Object> Xblab::pHandle_;
void Xblab::InitAll(Handle<Object> module) {
    try {
        // Start crypto on module load
        Botan::LibraryInitializer init("thread_safe=true");
    }
    catch(std::exception& e) {
        std::cerr << e.what() << "\n";
    }

    nodeBufCtor = JS_NODE_BUF_CTOR;

    // Create state-holding instance
    Xblab* instance = new Xblab();
    Local<ObjectTemplate> t = ObjectTemplate::New();
    t->SetInternalFieldCount(1);   
    Local<Object> holder = t->NewInstance();    
    instance->Wrap(holder);
    pHandle_ = Persistent<Object>::New(holder);

    Manager::Init(module);
    module->Set(String::NewSymbol("createManager"),
        FunctionTemplate::New(CreateManager)->GetFunction());

    module->Set(String::NewSymbol("config"),
        FunctionTemplate::New(SetConfig)->GetFunction());

    module->Set(String::NewSymbol("getConnectionBuffer"),
        FunctionTemplate::New(OnConnection)->GetFunction());

    module->Set(String::NewSymbol("digestBuffer"),
        FunctionTemplate::New(DigestBuffer)->GetFunction());   
}

Xblab::Xblab(){
    currentUsers_ = map<int, User>();
}


// Stand-in for Manager::New
Handle<Value> Xblab::CreateManager(const Arguments& args) {
    HandleScope scope;

    // Xblab* instance = ObjectWrap::Unwrap<Xblab>(pHandle_);
    // instance->proveExistence();
    // String::Utf8Value s(connstring->ToString());
    return scope.Close(Manager::NewInstance(args));
}


Handle<Value> Xblab::SetConfig(const Arguments& args) {

    HandleScope scope;

    // TODO: error callback
    if (!args[0]->IsObject()) {
        THROW("xblab.config requires object argument");
    }

    Handle<Object> cfg = Handle<Object>::Cast(args[0]);
    Handle<Value> constr = cfg->Get(String::New("connstring"));
    Handle<Value> pubkfile = cfg->Get(String::New("pubKeyFile"));
    Handle<Value> privkfile = cfg->Get(String::New("privKeyFile"));
    Handle<Value> keypw = cfg->Get(String::New("keyPassphrase"));


    connstring = NODE_PSYMBOL(*(String::Utf8Value(constr)));
    pub_key_filename = NODE_PSYMBOL(*(String::Utf8Value(pubkfile)));
    priv_key_filename = NODE_PSYMBOL(*(String::Utf8Value(privkfile)));
    key_passphrase = NODE_PSYMBOL(*(String::Utf8Value(keypw)));

    return scope.Close(Undefined());
}


Handle<Value> Xblab::OnConnection(const Arguments& args) {

    HandleScope scope;
    if (!args[0]->IsFunction()){
        THROW("xblab.getConnectionBuffer requires callback argument");
    }

    // Xblab* instance = ObjectWrap::Unwrap<Xblab>(pHandle_);
    // map<int, User>::const_iterator itr = instance->currentUsers_.begin();
    // for(; itr != instance->currentUsers_.end(); ++itr){
    //     cout << "user " << itr->first << ": " << itr->second.username << endl;
    // }

    Local<Function> cb = Local<Function>::Cast(args[0]);
    const unsigned argc = 2;
    Local<Value> argv[argc];

    try{
        string nonce;
        string buf = Util::needCredBuf(nonce); // serialized "NEEDCRED buffer     
        /*
            You could also use buf.data() for this next line,
            but C++11 strings are guaranteed to be
            allocated contiguously.
        */
        const char *c = &buf[0]; 
        size_t len = buf.size();

        // Send packet with transmission nonce and binary protocol buffer
        // Currently, responsibility for keeping track of unattached users lies with JS
        Local<Object> packet = Object::New();
        packet->Set(String::NewSymbol("nonce"), String::New(nonce.c_str()));
        packet->Set(String::NewSymbol("buffer"), Util::wrapBuf(c, len));

        argv[0] = Local<Value>::New(Undefined());
        argv[1] = packet;
        
    }
    catch (util_exception& e){
        argv[0] = Local<Value>::New(String::New(e.what()));
        argv[1] = Local<Value>::New(Undefined());
    }
    cb->Call(Context::GetCurrent()->Global(), argc, argv);
    return scope.Close(Undefined());
}



Handle<Value> Xblab::DigestBuffer(const Arguments& args) {
    HandleScope scope;

    if (!args[0]->IsObject() || !args[1]->IsFunction()){
        THROW("xblab: digestBuffer() requires buffer and callback argument");
    }

    Local<Object> packet = args[0]->ToObject();
    Local<Value> lastNonce = packet->Get(String::New("nonce"));
    Local<Value> buffer = packet->Get(String::New("buffer"));

    // Parse binary data from node::Buffer
    char* bufData = Buffer::Data(buffer->ToObject());
    int bufLen = Buffer::Length(buffer->ToObject());     

    string buf(bufData, bufData + bufLen);
    
    try {

        // TODO: how to compare nonces when user has not yet entered a group?

        // TODO: parseBuf needs to return a way for us to decide what
        // event to emit, and optionally some data for us to broadcast
        
        // Util is responsible for deciding when to create manager?
        // No, that should be done in JS, leaving us free to assume this is a pre-chat event
        // Types of events: get rooms, join chat


        // TODO: pass users
        Util::parseTransmission(Util::v8ToString(lastNonce), buf);
    }
    catch (util_exception& e){
        
        // TODO: should errors always be handled in user-supplied callback?
        Local<Function> cb = Local<Function>::Cast(args[1]);
        Local<Value> argv[1] = { String::New(e.what()) };
        cb->Call(Context::GetCurrent()->Global(), 1, argv);
    }

    return scope.Close(Undefined());
}

void Xblab::proveExistence(){
    cout << "I\'ve been unwrapped!\n";
    User u = User("austin", "nitsua");
    this->currentUsers_.insert(pair<int, User>(1, u));
}


/*
  Intenal class access from JS is difficult due to name mangling in C++
  Use extern C here to initalize module handle.
  Static class members are especially problematic, look for a workaround
*/
extern "C" {
  static void init(v8::Handle<v8::Object> module) {    
    xblab::Xblab::InitAll(module);
  }  
  NODE_MODULE(xblab, init);
}

} //namespace xblab

