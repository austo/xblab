#include <iostream>
#include <sstream>
#include <fstream>


// #include <node_buffer.h>

// #include <botan/botan.h>
// #include <botan/bcrypt.h>
// #include <botan/rsa.h>
// #include <botan/look_pk.h>

#include "binding/xbClient.h"
#include "binding/nodeUtil.h"
#include "native/client.h"
#include "macros.h"

using namespace std;
using namespace v8;
using namespace node;


namespace xblab {


string xbPublicKeyFile;
string xbServerAddress;
string xbServerPort;
Persistent<Function> xbNodeBufCtor;

uv_loop_t *loop;


/* instance member functions */

XbClient::XbClient(string group) {
  this->group_ = group;
  this->baton_ = NULL;  
}


~XbClient() {
  if (baton != NULL){
    delete baton;
  }
}


bool
XbClient::hasParticipant() {
  return this->baton_ != NULL;
}


void
XbClient::initializeBaton(uv_connect_cb cb) {
  if (!this->hasParticipant()){
    this->baton_ = new ParticipantBaton(cb);
  }
}


/* static member functions */

// args -> username, password, group TODO: make object/callback
Handle<Value>
XbClient::New(const Arguments& args){
  HandleScope scope; 

  XbClient* instance;
  if (!args[0]->IsObject()){
    THROW("xblab.XbClient requires configuration object.");
  }

  Handle<Object> cfg = Handle<Object>::Cast(args[0]);
  Handle<Value> pubkfile = GET_PROP(cfg, xbPublicKeyFile);
  Local<Value> group = cfg->Get(String::New(XBGROUP));

  xbPublicKeyFile = string(*(String::Utf8Value(pubkfile)));

  instance = new XbClient(NodeUtil::v8ToString(group));   

  instance->Wrap(args.This());
  return scope.Close(args.This());
}


Handle<Value>
XbClient::GetHandle(Local<String> property, const AccessorInfo& info){
  // Extract C++ request object from JS wrapper
  XbClient* instance = ObjectWrap::Unwrap<XbClient>(info.Holder());
  return String::New(instance->handle_.c_str());
}


void
XbClient::SetHandle(Local<String> property,
  Local<Value> value, const AccessorInfo& info){
  XbClient* instance =
    ObjectWrap::Unwrap<XbClient>(info.Holder());
  instance->handle_ = NodeUtil::v8ToString(value);
}


// TODO: error handling!
Handle<Value>
XbClient::SendCred(const Arguments& args) {
  HandleScope scope;

  if (!args[0]->IsObject() || !args[1]->IsFunction()) {
    THROW("xblab: sendCred() requires object and callback arguments");
  }

  Local<Object> credentials = Local<Object>::Cast(args[0]);
  Local<Value> username = credentials->Get(String::New(XBUSERNAME));
  Local<Value> password = credentials->Get(String::New(XBPASSWORD));

  XbClient* instance = ObjectWrap::Unwrap<XbClient>(args.This());
  instance->username_ = NodeUtil::v8ToString(username);
  instance->password_ = NodeUtil::v8ToString(password);

  string buf = Util::packageParticipantCredentials(instance);

  const char *c = &buf[0];
  size_t len = buf.size();

  Handle<Value> argv[2] = {
    String::New("haveCred"),
    NodeUtil::wrapBuf(c, len)
  };

  node::MakeCallback(args.This(), "emit", 2, argv);

  return scope.Close(Undefined());
}


Handle<Value>
XbClient::DigestBuffer(const Arguments& args) {
  HandleScope scope;

  if (!args[0]->IsObject() || !args[1]->IsFunction()){
    THROW("xblab: digestBuffer() requires buffer and callback argument");
  }

  // Parse binary data from node::Buffer
  char* bufData = Buffer::Data(args[0]->ToObject());
  int bufLen = Buffer::Length(args[0]->ToObject());    

  // copy node::Buffer contents into string
  string buf(bufData, bufData + bufLen);
  
  try {

    XbClient* instance = ObjectWrap::Unwrap<XbClient>(args.This());
    cout << "group: " << instance->group_ << endl;

    // TODO: parseBuf needs to return a way for us to decide what
    // event to emit, and optionally some data for us to broadcast
    
    MessageType messagetype = Util::parseBroadcast(buf, instance);
    if (messagetype == NEEDCRED){

      Handle<Value> argv[2] = {
        String::New("needCred"),
        String::New("Need credentials, buster!")
      };

      node::MakeCallback(args.This(), "emit", 2, argv);
    }

  }
  catch (util_exception& e){
    
    // TODO: should errors always be handled in user-supplied callback?
    Local<Function> cb = Local<Function>::Cast(args[1]);
    Local<Value> argv[1] = { String::New(e.what()) };
    cb->Call(Context::GetCurrent()->Global(), 1, argv);
  }

  return scope.Close(Undefined());
}




Handle<Value>
XbClient::Connect(const Arguments& args){
  HandleScope scope;
  if (!args[0]->IsFunction()){
    THROW("xblab.getConnectionBuffer requires callback argument");
  }

  // Error callback only
  Local<Function> cb = Local<Function>::Cast(args[0]);
  
  XbClient* instance = ObjectWrap::Unwrap<XbClient>(args.This());

  int port = atoi(xbPort.c_str());
  struct sockaddr_in xb_addr = uv_ip4_addr(xbServerAddress.c_str(), port);
  loop = uv_default_loop();

  instance->initializeBaton(Client::onConnect);  
  uv_tcp_connect(
    &instance->baton_->uvConnect,
    &instance->baton_->uvClient,
    xb_addr,
    instance->baton_->uvConnectCb
  );
  
  return scope.Close(Undefined());
}


extern "C" {

  // TODO: move majority of this code to XbClient::Init()
  void init(Handle<Object> module) {
    HandleScope scope;

    xbNodeBufCtor = JS_NODE_BUF_CTOR;
    loop = uv_default_loop();

    if (Crypto::init() != XBGOOD){
      THROW("XbClient: failed to initialize Crypto.");
    }

    Local<FunctionTemplate> t = FunctionTemplate::New(XbClient::New);
    t->InstanceTemplate()->SetInternalFieldCount(1);
    t->SetClassName(String::New("XbClient"));
    t->InstanceTemplate()->SetAccessor(String::New("handle"),
      XbClient::GetHandle, XbClient::SetHandle);

    // Only methods exposed to JS should go here, emitted events are "private"
    NODE_SET_PROTOTYPE_METHOD(t, "digestBuffer", XbClient::DigestBuffer);
    NODE_SET_PROTOTYPE_METHOD(t, "sendCred", XbClient::SendCred);

    module->Set(String::NewSymbol("XbClient"), t->GetFunction());        
  }   
  NODE_MODULE(xblab, init);
}

} //namespace xblab