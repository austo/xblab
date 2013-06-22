#include <iostream>
#include <sstream>

#include <node_buffer.h>

#include <botan/botan.h>
#include <botan/bcrypt.h>
#include <botan/rsa.h>
#include <botan/look_pk.h>

#include "participant.h"
#include "util.h"
#include "crypto.h"
#include "macros.h"


namespace xblab {

using namespace std;
using namespace v8;
using namespace node;

v8::Persistent<v8::String> pub_key_filename;
v8::Persistent<v8::Function> nodeBufCtor;


// args -> username, password, group TODO: make object/callback
Handle<Value> Participant::New(const Arguments& args){
    HandleScope scope; 

    Participant* instance;
    if (args[0]->IsUndefined()){
        instance = new Participant();
    }
    else {
        // args is object, use parameterized ctor
        instance = new Participant(
        Util::v8ToString(args[0]),
        Util::v8ToString(args[1]),
        Util::v8ToString(args[2]));
    }   

    instance->Wrap(args.This());
    return scope.Close(args.This());
}

Handle<Value> Participant::NeedCredentials(const Arguments& args) {
  HandleScope scope;

  Handle<Value> argv[2] = {
    String::New("cred"), // event name
    args[0]->ToString()  // argument
  };

  node::MakeCallback(args.This(), "emit", 2, argv);

  return Undefined();
}


Handle<Value>
Participant::GetHandle(Local<String> property, const AccessorInfo& info){
    // Extract C++ request object from JS wrapper
    Participant* instance = ObjectWrap::Unwrap<Participant>(info.Holder());
    return String::New(instance->handle_.c_str());
}


void
Participant::SetHandle(Local<String> property, Local<Value> value, const AccessorInfo& info){
    Participant* instance = ObjectWrap::Unwrap<Participant>(info.Holder());
    instance->handle_ = Util::v8ToString(value);
}

// TODO: error callback
Handle<Value> Participant::SetConfig(const Arguments& args) {
    HandleScope scope;

    if (!args[0]->IsObject()) {
            THROW("xblab: config() requires object argument");
    }

    Handle<Object> cfg = Handle<Object>::Cast(args[0]);
    Handle<Value> pubkfile = cfg->Get(String::New("pubKeyFile"));

    pub_key_filename = NODE_PSYMBOL(*(String::Utf8Value(pubkfile)));
    cout << *(String::Utf8Value(pub_key_filename)) << endl;

    return scope.Close(Undefined());
}


Handle<Value> Participant::DigestBuffer(const Arguments& args) {
    HandleScope scope;

    // Parse binary data from node::Buffer
    char* bufData = Buffer::Data(args[0]->ToObject());
    int bufLen = Buffer::Length(args[0]->ToObject());

    if (!args[1]->IsFunction()){
        THROW("xblab: digestBuffer() requires callback argument");
    }

    string buf(bufData, bufData + bufLen); // copy node::Buffer contents into string

    /*
        TODO: this next section should happen after we've formulated
        the appropriate response, which varies depending on the buffer
        we've just parsed. In any event, we're going to need to build
        the response buffer before returning to JS land.
    */
    // Local<Function> cb = Local<Function>::Cast(args[1]);
    //const unsigned argc = 2;

    //Handle<Value> argv[2];

    try{

        string bufferResult = Util::parseBuf(buf);
        Handle<Value> argv[2] = {
            String::New("cred"), // event name
            String::New(bufferResult.c_str())  // argument
        };

        node::MakeCallback(args.This(), "emit", 2, argv);

        // argv[0] = String::New("cred");
        // argv[1] = String::New(bufferResult.c_str());

        //argv[0] = Local<Value>::New(Undefined()); //Error

        /*
            TODO: if buffer is okay, get username and password,
            then initialize Participant
        */
        //argv[1] = Local<Value>::New(String::New(Util::parseBuf(buf).c_str()));

    }
    catch (util_exception& e){
        // // TODO: should be on error
        // argv[0] = String::New("error");

        Local<Function> cb = Local<Function>::Cast(args[1]);
        Local<Value> argv[1] = { String::New(e.what()) };
        cb->Call(Context::GetCurrent()->Global(), 1, argv);



        // argv[1] = String::New(e.what());
        // argv[0] = Local<Value>::New(String::New(e.what()));
        // argv[1] = Local<Value>::New(Undefined());
    }

    // argv[0] = Local<Value>::New(String::New(bufferResult.c_str()));


    


    return scope.Close(Undefined());
}


//TODO: Sign
//TODO: Decrypt


Participant::Participant(string username, string password, string group) :
    username_(username), password_(password), group_(group) {
    try{
        Crypto::generateKey(this->priv_key_, this->pub_key_); //most likely to fail        
    }
    catch(exception& e){
        cout << "Exception caught: " << e.what() << endl;
        throw;
    }
}

Participant::Participant() {
    try{
        Crypto::generateKey(this->priv_key_, this->pub_key_); //most likely to fail        
    }
    catch(exception& e){
        cout << "Exception caught: " << e.what() << endl;
        throw;
    }
}


extern "C"{
  void init(Handle<Object> target) {
    HandleScope scope;

    // TODO: move to Participant::Init()

    nodeBufCtor = JS_NODE_BUF_CTOR;

    try {
        // Start up crypto - happens only once per module load
        Botan::LibraryInitializer init("thread_safe=true");
    }
    catch(std::exception& e) {
        std::cerr << e.what() << "\n";
    }


    Local<FunctionTemplate> t = FunctionTemplate::New(Participant::New);
    t->InstanceTemplate()->SetInternalFieldCount(1);
    t->SetClassName(String::New("Participant"));
    t->InstanceTemplate()->SetAccessor(String::New("handle"),
        Participant::GetHandle, Participant::SetHandle);


    NODE_SET_PROTOTYPE_METHOD(t, "digestBuffer", Participant::DigestBuffer);
    // NODE_SET_PROTOTYPE_METHOD(t, "cred", Participant::NeedCredentials);

    // target->Set(String::NewSymbol("digestBuffer"),
    //     FunctionTemplate::New(Participant::DigestBuffer)->GetFunction());

    target->Set(String::NewSymbol("Participant"), t->GetFunction());

    target->Set(String::NewSymbol("config"),
        FunctionTemplate::New(Participant::SetConfig)->GetFunction());
    
  }
  NODE_MODULE(xblab, init);

}

} //namespace xblab

