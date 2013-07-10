#include <unistd.h>

#include <iostream>
#include <vector>
#include <botan/botan.h>

#include <node_buffer.h>

#include "macros.h"
#include "binding/server.h"
#include "binding/manager.h"
#include "binding/util.h"
#include "binding/nodeUtil.h"
#include "binding/clientBaton.h"


using namespace v8;
using namespace node;
using namespace std;


namespace xblab {


// We use node::Buffer enough to keep its constructor handy
v8::Persistent<v8::Function> xbNodeBufCtor;

/* Global configuration items */
string xbConnectionString;
string xbPrivateKeyFile;
string xbPublicKeyFile;
string xbKeyPassword;

// V8 entry point
// Repeat declaration of static member(s) to make visible to node dlopen
v8::Persistent<v8::Object> Xblab::pHandle_;
void Xblab::InitAll(Handle<Object> module) {
  try {
    // Start crypto on module load
    Botan::LibraryInitializer init("thread_safe=true");
  }
  catch(std::exception& e) {
    std::cerr << e.what() << "\n";
  }

  xbNodeBufCtor = JS_NODE_BUF_CTOR;

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
  ClientBaton *baton = new ClientBaton(cb);    
  uv_queue_work(uv_default_loop(), &baton->uvWork,
    OnConnectWork, (uv_after_work_cb)AfterOnConnect);
  
  return scope.Close(Undefined());
}


void Xblab::OnConnectWork(uv_work_t *r){
  ClientBaton *baton = reinterpret_cast<ClientBaton *>(r->data);

  string nonce;
  // get serialized "NEEDCRED buffer
  string buf = Util::needCredBuf(nonce);
  baton->nonce = nonce;
  baton->xBuffer = buf;
}


void Xblab::AfterOnConnect (uv_work_t *r) {
  HandleScope scope;
  ClientBaton *baton = reinterpret_cast<ClientBaton *>(r->data);

  TryCatch tc;

  const unsigned argc = 2;
  Local<Value> argv[argc];


  const char *c = &(baton->xBuffer[0]); 
  size_t len = baton->xBuffer.size();

  Local<Object> packet = Object::New();    
  packet->Set(String::NewSymbol("nonce"), String::New(baton->nonce.c_str()));
  packet->Set(String::NewSymbol("buffer"), NodeUtil::wrapBuf(c, len));

  argv[0] = Local<Value>::New(Undefined());
  argv[1] = packet;

  baton->callback->Call(Context::GetCurrent()->Global(), argc, argv);

  delete baton;

  if (tc.HasCaught()) {
    FatalException(tc);
  }
}


Handle<Value> Xblab::SetConfig(const Arguments& args) {

  HandleScope scope;

  // TODO: error callback
  if (args.Length() != 2 ||
    !args[0]->IsObject() || !args[1]->IsFunction()) {
    THROW("xblab.config requires object and callback arguments");
  }

  Handle<Object> cfg = Handle<Object>::Cast(args[0]);
  Local<Function> cb = Local<Function>::Cast(args[1]);    

  Handle<Value> constr = GET_PROP(cfg, xbConnectionString);
  Handle<Value> pubkfile = GET_PROP(cfg, xbPublicKeyFile);
  Handle<Value> privkfile = GET_PROP(cfg, xbPrivateKeyFile);
  Handle<Value> keypw = GET_PROP(cfg, xbKeyPassword);

  if (constr->IsUndefined() || pubkfile->IsUndefined() ||
    privkfile->IsUndefined() || keypw->IsUndefined()){
    Handle<Value> argv[1] = { 
      String::New("xblab.config: one or more properties is undefined.")
    };
    cb->Call(Context::GetCurrent()->Global(), 1, argv);
    return scope.Close(Undefined());
  }
  
  xbConnectionString = string(*(String::Utf8Value(constr)));
  xbPrivateKeyFile = string(*(String::Utf8Value(privkfile)));
  xbPublicKeyFile = string(*(String::Utf8Value(pubkfile)));
  xbKeyPassword = string(*(String::Utf8Value(keypw)));
  
  return scope.Close(Undefined());
}


Handle<Value> Xblab::DigestBuf(const Arguments& args) {
  HandleScope scope;
  const unsigned argc = 2;

  if (!args[0]->IsObject() || !args[1]->IsFunction()){
    THROW("xblab: digestBuffer() requires buffer and callback argument");
  }


  Local<Object> packet = args[0]->ToObject();
  Local<Value> lastNonce = packet->Get(String::New("nonce"));
  Local<Value> buffer = packet->Get(String::New("buffer"));

  Local<Function> cb = Local<Function>::Cast(args[1]);

  if (lastNonce->IsUndefined() || buffer->IsUndefined()){
    Handle<Value> argv[argc] = {
      String::New("Invalid arguments"),
      Null()
    };
    cb->Call(Context::GetCurrent()->Global(), argc, argv);
    return scope.Close(Undefined());
  }    

  // Parse binary data from node::Buffer
  char* bufData = Buffer::Data(buffer->ToObject());
  int bufLen = Buffer::Length(buffer->ToObject());     

  string buf(bufData, bufData + bufLen);
  
  try {

    Xblab *instance = ObjectWrap::Unwrap<Xblab>(pHandle_);

    ClientBaton *baton = new ClientBaton(cb);
    baton->nonce = NodeUtil::v8ToString(lastNonce);
    baton->xBuffer = buf;        
    baton->auxData = &instance->mptrs;

    uv_queue_work(uv_default_loop(), &baton->uvWork,
      DigestBufWork, (uv_after_work_cb)AfterDigestBuf);
    
  }
  catch (util_exception& e){
    
    // TODO: should errors always be handled in user-supplied callback?
    Local<Value> argv[1] = { String::New(e.what()) };
    cb->Call(Context::GetCurrent()->Global(), 1, argv);
  }

  return scope.Close(Undefined());
}

// TODO: quite imcomplete: need a way of determining proper action
// or transfering ownership to Manager
void Xblab::DigestBufWork(uv_work_t *r){
  try{
    ClientBaton *baton = reinterpret_cast<ClientBaton *>(r->data);
    baton->err = "";
    Util::unpackMember(baton);
  }
  catch (util_exception& e){
    cout << e.what();
  }
}


// TODO: callback!
void Xblab::AfterDigestBuf(uv_work_t *r) {
  HandleScope scope;
  ClientBaton *baton = reinterpret_cast<ClientBaton *>(r->data);

  TryCatch tc;

  // const unsigned argc = 2;
  // Local<Value> argv[argc];

  if (baton->err == ""){
    Xblab* instance = ObjectWrap::Unwrap<Xblab>(pHandle_);
    if (instance->Managers.find(baton->url) == instance->Managers.end()){
      // add new manager from mptrs
      Local<ObjectTemplate> t = ObjectTemplate::New();
      t->SetInternalFieldCount(1);
      Local<Object> holder = t->NewInstance();

      Manager *mgr = reinterpret_cast<Manager*>(instance->mptrs.at(baton->url));

      mgr->Wrap(holder);
      instance->Managers.insert(
        pair<string, Handle<Object> >(baton->url, mgr->handle_));
    }
  }

  // TODO: return welcome buffer
  delete baton;

  if (tc.HasCaught()) {
    FatalException(tc);
  }
}


extern "C" {
  static void init(v8::Handle<v8::Object> module) {    
  xblab::Xblab::InitAll(module);
  }  
  NODE_MODULE(xblab, init);
}

} //namespace xblab

