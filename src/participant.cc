#include <iostream>
#include <sstream>

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
using namespace Botan;

Persistent<Function> Participant::constructor;
void Participant::Init(){
    // Constructor function template
    Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
    tpl->SetClassName(String::NewSymbol("Participant"));
    tpl->InstanceTemplate()->SetInternalFieldCount(1);    
    tpl->InstanceTemplate()->SetAccessor(String::New("handle"), GetHandle, SetHandle);
    //NODE_SET_PROTOTYPE_METHOD(tpl, "pogo", Pogo);


    // Prototype    
    constructor = Persistent<Function>::New(tpl->GetFunction());
}

// Provides static access to JS new
// TODO: args should be object -> parse and give to New
Handle<Value> Participant::NewInstance(const Arguments& args){
    HandleScope scope;
    const unsigned argc = 1;
    Handle<Value> argv[argc] = { args[0] };
    Local<Object> instance = constructor->NewInstance(argc, argv);
    return scope.Close(instance);
}

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

Handle<Value> Participant::Pogo(const Arguments& args) {
  HandleScope scope;

  Handle<Value> argv[2] = {
    String::New("pogo"), // event name
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

//TODO: Encrypt should take participant token, then encrypt with participant public key
//TODO: delete PK_Encryptor/decryptor
void Participant::BuildPacket(string in){
    AutoSeeded_RNG rng;

#ifdef __DEBUG
    cout << this->pub_key_ << endl;
#endif

    DataSource_Memory ds_pub(this->pub_key_);
    //DataSource_Memory ds_priv(this->priv_key_);

    X509_PublicKey *pub_rsa = X509::load_key(ds_pub);
    //PKCS8_PrivateKey *priv_rsa = PKCS8::load_key(ds_priv, rng);

    PK_Encryptor *enc = new PK_Encryptor_EME(*pub_rsa, SHA256);
    //PK_Decryptor *dec = new PK_Decryptor_EME(*priv_rsa, SHA256);

    vector<byte> bytes(in.begin(), in.end());
    byte *c = &bytes[0];
        
    SecureVector<byte> ciphertext = enc->encrypt(c, bytes.size(), rng);
    Pipe pipe(new Base64_Encoder);
    pipe.process_msg(ciphertext);
    //pipe.write(ciphertext);
    //SecureVector<byte> plaintext = dec->decrypt(ciphertext, ciphertext.size());

    //string res(ciphertext.begin(), ciphertext.end());
    cout << "output: " << pipe.read_all_as_string() << endl;
}

void Participant::DigestPacket(string in){

}

} //namespace xblab
