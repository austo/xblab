#include <iostream>
#include <sstream>
#include <fstream>

#include "binding/xbClient.h"
#include "binding/nodeUtil.h"
#include "native/client.h"
#include "native/participantUtil.h"
#include "macros.h"
#include "crypto.h"

using namespace std;
using namespace v8;
using namespace node;


namespace xblab {


extern void on_connect(uv_connect_t *req, int status);    


string xbPublicKeyFile;
string xbServerAddress;
string xbServerPort;
Persistent<Function> xbNodeBufCtor;

Persistent<Object> XbClient::pHandle_;


uv_loop_t *loop;


/* instance member functions */

XbClient::XbClient(string group) {
  this->group_ = group;
  this->baton_ = new ParticipantBaton();
  this->baton_->url = group; 
}


XbClient::~XbClient() {
  if (this->baton_ != NULL){
    delete baton_;
  }
  this->pHandle_.Dispose();
}


bool
XbClient::hasParticipant() {
  return this->baton_ != NULL;
}


void
XbClient::initializeBaton() {
  if (!this->hasParticipant()){
    this->baton_ = new ParticipantBaton();
    this->baton_->wrapper = this;
  }  
}


Handle<Value>
XbClient::emitRequestCredential(){
  HandleScope scope;

  try {

    Handle<Value> argv[2] = {
      String::New("needCred"),
      String::New("Need credentials, buster!")
    };

    node::MakeCallback(this->pHandle_, "emit", 2, argv);
  }

  catch (util_exception& e) {
    Handle<Value> argv[2] = {
      String::New("error"),
      String::New(e.what())
    };
    node::MakeCallback(this->pHandle_, "emit", 2, argv);
  }

  return scope.Close(Undefined());  
}


Handle<Value>
XbClient::emitGroupEntry(){
  HandleScope scope;

  try {
    char buf[30];
    sprintf(buf, "Welcome to %s.", baton_->url.c_str());

    Handle<Value> argv[2] = {
      String::New("groupEntry"),
      String::New(buf)
    };

    node::MakeCallback(pHandle_, "emit", 2, argv);
  }

  catch (util_exception& e) {
    Handle<Value> argv[2] = {
      String::New("error"),
      String::New(e.what())
    };
    node::MakeCallback(this->pHandle_, "emit", 2, argv);
  }

  return scope.Close(Undefined());  
}

/* static member functions */

Handle<Value>
XbClient::New(const Arguments& args){
  HandleScope scope; 

  XbClient* instance;
  if (!args[0]->IsObject()){
    THROW("xblab.XbClient requires configuration object.");
  }

  Handle<Object> cfg = Handle<Object>::Cast(args[0]);

  Handle<Value> pubkfile = GET_PROP(cfg, xbPublicKeyFile);
  Handle<Value> servaddr = GET_PROP(cfg, xbServerAddress);
  Handle<Value> servport = GET_PROP(cfg, xbServerPort);

  Local<Value> group = cfg->Get(String::New(XBGROUP));

  xbPublicKeyFile = string(*(String::Utf8Value(pubkfile)));
  xbServerAddress = string(*(String::Utf8Value(servaddr)));
  xbServerPort = string(*(String::Utf8Value(servport)));

  instance = new XbClient(NodeUtil::v8ToString(group));   

  instance->Wrap(args.This());
  pHandle_ = Persistent<Object>::New(args.This());
  return scope.Close(args.This());
}


Handle<Value>
XbClient::GetHandle(Local<String> property, const AccessorInfo& info){
  XbClient* instance = ObjectWrap::Unwrap<XbClient>(info.Holder());
  return String::New(instance->baton_->participant.handle.c_str());
}


void
XbClient::SetHandle(Local<String> property,
  Local<Value> value, const AccessorInfo& info){
  XbClient* instance =
    ObjectWrap::Unwrap<XbClient>(info.Holder());
  instance->baton_->participant.handle = NodeUtil::v8ToString(value);
}


// TODO: error handling!
Handle<Value>
XbClient::SendCredential(const Arguments& args) {
  HandleScope scope;

  if (!args[0]->IsObject() || !args[1]->IsFunction()) {
    THROW("xblab: sendCred() requires object and callback arguments");
  }

  Local<Object> credentials = Local<Object>::Cast(args[0]);
  Local<Value> username = credentials->Get(String::New(XBUSERNAME));
  Local<Value> password = credentials->Get(String::New(XBPASSWORD));

  XbClient* instance = ObjectWrap::Unwrap<XbClient>(args.This());
  instance->baton_->wrapper = instance; // seems to be necessary

  instance->baton_->participant.username = NodeUtil::v8ToString(username);
  instance->baton_->participant.password = NodeUtil::v8ToString(password);

  Client::onSendCredential(instance->baton_);
  
  return scope.Close(Undefined());
}


// C++ -> JS event emission wrappers
Handle<Value>
XbClient::requestCredentialFactory(XbClient *xbClient) {
  return xbClient->emitRequestCredential();
}


Handle<Value>
XbClient::groupEntryFactory(XbClient *xbClient) {
  cout << xbClient->baton_->url;
  return xbClient->emitGroupEntry();
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

  int port = atoi(xblab::xbServerPort.c_str());
  struct sockaddr_in xb_addr = uv_ip4_addr(xbServerAddress.c_str(), port);
  loop = uv_default_loop();

  instance->initializeBaton(); 

  uv_tcp_init(loop, &instance->baton_->uvClient);

  int status = uv_tcp_connect(
    &instance->baton_->uvConnect,
    &instance->baton_->uvClient,
    xb_addr,
    on_connect
  );

  Handle<Value> argv[1];
  if (status == XBGOOD){
    argv[0] = Undefined();
  }
  else{
      argv[0] = String::New("Unable to connect to xblab server.");
  }
  cb->Call(Context::GetCurrent()->Global(), 1, argv);
  return scope.Close(Undefined());
}

} //namespace xblab

extern "C" { 

  void init(Handle<Object> module) {
    HandleScope scope;

    xblab::xbNodeBufCtor = JS_NODE_BUF_CTOR;
    xblab::loop = uv_default_loop();

    if (xblab::Crypto::init() != XBGOOD){
      THROW("XbClient: failed to initialize Crypto.");
    }

    Local<FunctionTemplate> t = FunctionTemplate::New(xblab::XbClient::New);
    t->InstanceTemplate()->SetInternalFieldCount(1);
    t->SetClassName(String::New("Client"));
    t->InstanceTemplate()->SetAccessor(String::New("handle"),
      xblab::XbClient::GetHandle, xblab::XbClient::SetHandle);

     // Only methods exposed to JS should go here, emitted events are "private"
    NODE_SET_PROTOTYPE_METHOD(
      t, "connect", xblab::XbClient::Connect);
    NODE_SET_PROTOTYPE_METHOD(
      t, "sendCredential", xblab::XbClient::SendCredential);
    module->Set(String::NewSymbol("Client"), t->GetFunction());        
  }

  NODE_MODULE(xblab, init);
}