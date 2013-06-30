#include <unistd.h>

#include <iostream>
#include <vector>
#include <botan/botan.h>

#include <node_buffer.h>

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
        FunctionTemplate::New(OnConnect)->GetFunction());

    module->Set(String::NewSymbol("digestBuffer"),
        FunctionTemplate::New(DigestBuf)->GetFunction());    
}

Xblab::Xblab(){ }


// Stand-in for Manager::New
Handle<Value> Xblab::CreateManager(const Arguments& args) {
    HandleScope scope;   
    return scope.Close(Manager::NewInstance(args));
}


Handle<Value> Xblab::OnConnect(const Arguments& args) {

    HandleScope scope;
    if (!args[0]->IsFunction()){
        THROW("xblab.getConnectionBuffer requires callback argument");
    }

    Local<Function> cb = Local<Function>::Cast(args[0]);
    
    // Populate baton struct to pass to uv_queue_work:
    // Private key and password unpacked here to avoid memory conflicts with v8 heap.
    DataBaton *baton = new DataBaton(cb);
    baton->privateKeyFile = string(*(v8::String::Utf8Value(priv_key_filename)));
    baton->password = string(*(v8::String::Utf8Value(key_passphrase)));

    uv_queue_work(uv_default_loop(), &baton->request,
        OnConnectWork, (uv_after_work_cb)AfterOnConnect);
    
    return scope.Close(Undefined());
}


void Xblab::OnConnectWork(uv_work_t *r){
    DataBaton *baton = reinterpret_cast<DataBaton *>(r->data);

    string nonce;
    // get serialized "NEEDCRED buffer
    string buf =
        Util::needCredBuf(baton->privateKeyFile, baton->password, nonce);
    baton->nonce = nonce;
    baton->buf = buf;
}


void Xblab::AfterOnConnect (uv_work_t *r) {
    HandleScope scope;
    DataBaton *baton = reinterpret_cast<DataBaton *>(r->data);

    TryCatch try_catch;

    const unsigned argc = 2;
    Local<Value> argv[argc];


    const char *c = &(baton->buf[0]); 
    size_t len = baton->buf.size();

    Local<Object> packet = Object::New();    
    packet->Set(String::NewSymbol("nonce"), String::New(baton->nonce.c_str()));
    packet->Set(String::NewSymbol("buffer"), Util::wrapBuf(c, len));

    argv[0] = Local<Value>::New(Undefined());
    argv[1] = packet;

    baton->callback->Call(Context::GetCurrent()->Global(), argc, argv);

    delete baton;

    if (try_catch.HasCaught()) {
        FatalException(try_catch);
    }
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


Handle<Value> Xblab::DigestBuf(const Arguments& args) {
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

        Local<Function> cb = Local<Function>::Cast(args[0]);

        DataBaton *baton = new DataBaton(cb);
        baton->nonce = Util::v8ToString(lastNonce);
        baton->buf = buf;
        baton->privateKeyFile = string(*(v8::String::Utf8Value(priv_key_filename)));
        baton->password = string(*(v8::String::Utf8Value(key_passphrase)));

        uv_queue_work(uv_default_loop(), &baton->request,
        DigestBufWork, (uv_after_work_cb)AfterDigestBuf);
        
    }
    catch (util_exception& e){
        
        // TODO: should errors always be handled in user-supplied callback?
        Local<Function> cb = Local<Function>::Cast(args[1]);
        Local<Value> argv[1] = { String::New(e.what()) };
        cb->Call(Context::GetCurrent()->Global(), 1, argv);
    }

    return scope.Close(Undefined());
}


void Xblab::DigestBufWork(uv_work_t *r){
    try{
        DataBaton *baton = reinterpret_cast<DataBaton *>(r->data);
        // TODO: need a way of determining proper action
        Util::unpackMember(baton);
    }
    catch (util_exception& e){
        cout << e.what();
    }
}

void Xblab::AfterDigestBuf (uv_work_t *r) {
    HandleScope scope;
    DataBaton *baton = reinterpret_cast<DataBaton *>(r->data);

    TryCatch try_catch;

    // const unsigned argc = 2;
    // Local<Value> argv[argc];

    Xblab* instance = ObjectWrap::Unwrap<Xblab>(pHandle_);

    // TODO: We are potentially creating a manager here - should be done in thread
    Util::addMember(baton, instance->Managers);

    // TODO: return welcome buffer
    delete baton;

    if (try_catch.HasCaught()) {
        FatalException(try_catch);
    }
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

