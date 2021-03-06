#include <iostream>
#include <sstream>
#include <fstream>

#include "client/binding/xbClient.h"
#include "client/binding/nodeUtil.h"
#include "client/client.h"
#include "common/util_exception.h"
#include "common/macros.h"
#include "common/common.h"
#include "common/crypto.h"

using namespace std;
using namespace v8;
using namespace node;

namespace xblab {

extern void on_connect(uv_connect_t *req, int status);
extern void on_close(uv_handle_t* handle);

string xbPublicKeyFile;
string xbServerAddress;
string xbServerPort;
Persistent<Function> xbNodeBufCtor;

Persistent<Object> XbClient::pHandle_;

uv_loop_t *loop;


/* instance member functions */

XbClient::XbClient(string group) {
  this->group_ = group;
  this->baton = new MemberBaton();
  this->baton->url = group; 
}


XbClient::~XbClient() {
  if (this->baton != NULL){
    delete baton;
  }
  this->pHandle_.Dispose();
}


bool
XbClient::hasMember() {
  return this->baton != NULL;
}


void
XbClient::initializeBaton() {
  if (!this->hasMember()) {
    this->baton = new MemberBaton();
    this->baton->wrapper = this;
  }  
}


Handle<Value>
XbClient::emitRequestCredential() {
  HandleScope scope;

  try {

    Handle<Value> argv[XBEMITARGS] = {
      String::New("needCred"),
      String::New("Need credentials, buster!")
    };
    XBEMITCALLBACK(pHandle_, argv);
  }

  catch (util_exception& e) {
    Handle<Value> argv[XBEMITARGS] = {
      String::New("error"),
      String::New(e.what())
    };
    XBEMITCALLBACK(pHandle_, argv);
  }

  return scope.Close(Undefined());  
}


Handle<Value>
XbClient::emitGroupEntry() {
  HandleScope scope;

  try {
    char buf[30];
    sprintf(buf, "Welcome to %s.", baton->url.c_str());

    Handle<Value> argv[XBEMITARGS] = {
      String::New("groupEntry"),
      String::New(buf)
    };
    XBEMITCALLBACK(pHandle_, argv);
  }

  catch (util_exception& e) {
    Handle<Value> argv[2] = {
      String::New("error"),
      String::New(e.what())
    };
    XBEMITCALLBACK(pHandle_, argv);
  }
  return scope.Close(Undefined());  
}


Handle<Value>
XbClient::emitEndConnection() {
  HandleScope scope;

  try {
    char buf[60];
    sprintf(buf, "%sConnection to %s at xblab server closed",
      rightnow().c_str(), baton->url.c_str());

    Handle<Value> argv[XBEMITARGS] = {
      String::New("end"),
      String::New(buf)
    };

    delete baton;
    baton = NULL;

    XBEMITCALLBACK(pHandle_, argv);
  }

  catch (util_exception& e) {
    Handle<Value> argv[XBEMITARGS] = {
      String::New("error"),
      String::New(e.what())
    };
    XBEMITCALLBACK(pHandle_, argv);
  }

  return scope.Close(Undefined());  
}

v8::Handle<v8::Value>
XbClient::emitStartChat() {
  HandleScope scope;

  try {
      char buf[30];
      sprintf(buf, "Begin chat");

      Handle<Value> argv[XBEMITARGS] = {
        String::New("beginChat"),
        String::New(buf)
      };
      XBEMITCALLBACK(pHandle_, argv);
    }

    catch (util_exception& e) {
      Handle<Value> argv[2] = {
        String::New("error"),
        String::New(e.what())
      };
      XBEMITCALLBACK(pHandle_, argv);
    }
    return scope.Close(Undefined());
}


v8::Handle<v8::Value>
XbClient::emitBroadcast() {
  HandleScope scope;

  try {
      char buf[30];
      sprintf(buf, "Begin chat");

      Handle<Value> argv[XBEMITARGS] = {
        String::New("broadcast"),
        String::New(buf)
      };
      XBEMITCALLBACK(pHandle_, argv);
    }

    catch (util_exception& e) {
      Handle<Value> argv[2] = {
        String::New("error"),
        String::New(e.what())
      };
      XBEMITCALLBACK(pHandle_, argv);
    }
    return scope.Close(Undefined());
}

/* static member functions */

// NOTE: look into converting object args to array Gets
// for use with JS shim
Handle<Value>
XbClient::New(const Arguments& args) {
  HandleScope scope; 

  XbClient* instance;
  if (!args[0]->IsObject()){
    THROW("xblab.XbClient requires configuration object");
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
XbClient::GetHandle(Local<String> property, const AccessorInfo& info) {
  XbClient* instance = ObjectWrap::Unwrap<XbClient>(info.Holder());
  return String::New(instance->baton->member.handle.c_str());
}


void
XbClient::SetHandle(Local<String> property,
  Local<Value> value, const AccessorInfo& info) {
  XbClient* instance =
    ObjectWrap::Unwrap<XbClient>(info.Holder());
  instance->baton->member.handle = NodeUtil::v8ToString(value);
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
  instance->baton->wrapper = instance; // seems to be necessary

  instance->baton->member.username = NodeUtil::v8ToString(username);
  instance->baton->member.password = NodeUtil::v8ToString(password);

  Client::onSendCredential(instance->baton);
  
  return scope.Close(Undefined());
}

// TODO: should be called Transmit Message if not handling files
Handle<Value>
XbClient::Transmit(const Arguments& args) {
  HandleScope scope;

  if (!args[0]->IsString() || !args[1]->IsFunction()) {
    THROW("xblab: transmit() requires sting and callback arguments");
  }

  // Local<Object> transmission = Local<Object>::Cast(args[0]);
  Local<Value> pload = args[0]->ToString();

  XbClient* instance = ObjectWrap::Unwrap<XbClient>(args.This());
  instance->baton->wrapper = instance; // seems to be necessary

  // TODO: add lock
  instance->baton->member.message = NodeUtil::v8ToString(pload);
  instance->baton->member.hasMessage = true;
  // Should be able to hold onto this message until it's our turn to send
  // Client::onTransmit(instance->baton);
  
  return scope.Close(Undefined());
}

// C++ -> JS event emission wrappers
Handle<Value>
XbClient::requestCredentialFactory(XbClient *xbClient) {
  return xbClient->emitRequestCredential();
}


Handle<Value>
XbClient::groupEntryFactory(XbClient *xbClient) {
  cout << rightnow() << "entering group "
    << xbClient->baton->url << endl;
  return xbClient->emitGroupEntry();
}


Handle<Value>
XbClient::endConnectionFactory(XbClient *xbClient) {
  // cout << xbClient->baton->url;
  return xbClient->emitEndConnection();
}

v8::Handle<v8::Value>
XbClient::startChatFactory(XbClient *xbClient) {
  return xbClient->emitStartChat();
}

v8::Handle<v8::Value>
XbClient::broadcastMessageFactory(XbClient *xbClient) {
  return xbClient->emitBroadcast();
}


Handle<Value>
XbClient::Connect(const Arguments& args) {
  HandleScope scope;
  if (!args[0]->IsFunction()) {
    THROW("xblab.getConnectionBuffer requires callback argument");
  }

  // Error callback only
  Local<Function> cb = Local<Function>::Cast(args[0]);
  
  XbClient* instance = ObjectWrap::Unwrap<XbClient>(args.This());

  int port = atoi(xblab::xbServerPort.c_str());
  struct sockaddr_in xb_addr = uv_ip4_addr(xbServerAddress.c_str(), port);
  loop = uv_default_loop();

  instance->initializeBaton(); 

  uv_tcp_init(loop, &instance->baton->uvClient);

  int status = uv_tcp_connect(
    &instance->baton->uvConnect,
    &instance->baton->uvClient,
    xb_addr,
    on_connect
  );

  Handle<Value> argv[1];
  if (status == XBGOOD) {
    argv[0] = Undefined();
  }
  else {
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

    if (xblab::Crypto::init() != XBGOOD) {
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
    NODE_SET_PROTOTYPE_METHOD(
      t, "transmit", xblab::XbClient::Transmit);
    module->Set(String::NewSymbol("Client"), t->GetFunction());   

  }

  NODE_MODULE(xblab, init);
}