#include "participant.h"
#include "util.h"
#include "macros.h"
#include "crypto.h"
#include <iostream>
#include <sstream>
#include <botan/botan.h>
#include <botan/bcrypt.h>
#include <botan/rsa.h>
#include <botan/look_pk.h>


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

    // Prototype    
    constructor = Persistent<Function>::New(tpl->GetFunction());
}

// Provides static access to JS new
Handle<Value> Participant::NewInstance(const Arguments& args){
    HandleScope scope;
    const unsigned argc = 3;
    Handle<Value> argv[argc] = { args[0], args[1], args[2] };
    Local<Object> instance = constructor->NewInstance(argc, argv);
    return scope.Close(instance);
}

// args -> username, password, group TODO: make object/callback
Handle<Value> Participant::New(const Arguments& args){
    HandleScope scope; 
    if (args.Length() != 3 || args[0]->IsUndefined()
        || args[1]->IsUndefined() || args[2]->IsUndefined()){
        THROW("Participant must be initialized with username, password, group.");
    }

    Participant* instance = new Participant(
        Util::v8_to_string(args[0]),
        Util::v8_to_string(args[1]),
        Util::v8_to_string(args[2]));

    instance->Wrap(args.This());
    return args.This();
}


Handle<Value> Participant::GetHandle(Local<String> property, const AccessorInfo& info){
    // Extract C++ request object from JS wrapper
    Participant* instance = ObjectWrap::Unwrap<Participant>(info.Holder());
    return String::New(instance->handle_.c_str());
}


void Participant::SetHandle(Local<String> property, Local<Value> value, const AccessorInfo& info){
    Participant* instance = ObjectWrap::Unwrap<Participant>(info.Holder());
    instance->handle_ = Util::v8_to_string(value);
}

//TODO: Sign
//TODO: Decrypt


Participant::Participant(string username, string password, string group) :
    username_(username), password_(password), group_(group) {
    try{
        Crypto::generate_key(this->priv_key_, this->pub_key_); //most likely to fail        
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
