#include <iostream>
#include <sstream>
#include <vector>

#include <node_buffer.h>

#include <botan/botan.h>
#include <botan/bcrypt.h>
#include <botan/rsa.h>
#include <botan/pubkey.h>
#include <botan/look_pk.h>

#include "macros.h"
#include "util.h"
#include "crypto.h"
#include "participant.h"


using namespace std;
using namespace Botan;


#define DL_EX_PREFIX "Util: "


namespace xblab {

extern v8::Persistent<v8::Function> nodeBufCtor;

Util::Util(){ /* The goal is to keep this a "static" class */ }
Util::~Util(){}

#ifndef XBLAB_CLIENT

string Util::needCredBuf(){    
    string nonce = Crypto::generateNonce();
    Broadcast bc;
    Broadcast::Data *data = new Broadcast::Data();

    data->set_type(Broadcast::NEEDCRED);
    data->set_nonce(nonce);

    string sig, datastr;
    if (!data->SerializeToString(&datastr)) {
        throw util_exception("Failed to serialize broadcast data.");
    }
    try{
        sig = Crypto::sign(datastr);
        bc.set_signature(sig);
        bc.set_allocated_data(data); //bc now owns data - no need to free
    }
    catch(crypto_exception& e){
        cout << "crypto exception: " << e.what() << endl;
    }

    string retval;
    if (!bc.SerializeToString(&retval)){
        throw util_exception("Failed to serialize broadcast.");
    }

    return retval;
}

#endif

string Util::packageParticipantCredentials(void* auxData){
    Participant* participant = (Participant *) auxData;
    string nonce = Crypto::generateNonce();
    Transmission trans;

    Transmission::Data *data = new Transmission::Data();
    Transmission::Credential *cred = new Transmission::Credential();

    data->set_type(Transmission::CRED);
    data->set_nonce(nonce);
    data->set_return_nonce(participant->return_nonce_);

    string passhash = Crypto::hashPassword(participant->password_);

    cred->set_username(participant->username_);
    cred->set_password(passhash);
    cred->set_pub_key(participant->pub_key_);

    data->set_allocated_credential(cred);


    string sig, datastr;
    if (!data->SerializeToString(&datastr)) {
        throw util_exception("Failed to serialize broadcast data.");
    }
    try{
        sig = Crypto::sign(participant->priv_key_, datastr);
        trans.set_signature(sig);
        trans.set_allocated_data(data);
    }
    catch(crypto_exception& e){
        cout << "crypto exception: " << e.what() << endl;
    }

    stringstream plaintext, ciphertext;
    if (!trans.SerializeToOstream(&plaintext)){
        throw util_exception("Failed to serialize broadcast.");
    }

    // cout << "\ntrans - DebugString:\n" << trans.DebugString() << endl;
    // cout << "\nplaintext str():\n" << plaintext.str() << endl;

    Crypto::hybridEncrypt(plaintext, ciphertext);

    return ciphertext.str();
}

// This is mainly for consumption by the client, so return Broadcast::Type
MessageType Util::parseBroadcast(string& in, void* auxData){

    MessageType retval = INVALID;
    //TODO: switch on broadcast type
    Broadcast bc;
    if (!bc.ParseFromString(in)){
        throw util_exception("Failed to deserialize broadcast.");
    }    

    string content;
    if (!bc.data().SerializeToString(&content)){
        throw util_exception("Failed to reserialize broadcast data.");
    }

    if (Crypto::verify(content, bc.signature())){
        cout << "Hooray!\n";
        const Broadcast::Data& data = bc.data();

        Participant* participant = (Participant *) auxData;
        participant->return_nonce_ = string(data.nonce());

        // TODO: no casts - proper type interrogation
        retval = (MessageType) data.type();
    }       

    return retval;
}

v8::Local<v8::Value> Util::wrapBuf(const char *c, size_t len){
    v8::HandleScope scope;
    static const unsigned bufArgc = 3;

    node::Buffer *slowBuffer = node::Buffer::New(len);        
    memcpy(node::Buffer::Data(slowBuffer), c, len); // Buffer::Data = (void *)    
   
    v8::Handle<v8::Value> bufArgv[bufArgc] = { 
        slowBuffer->handle_, // JS SlowBuffer handle
        v8::Integer::New(len), // SlowBuffer length
        v8::Integer::New(0) // Offset where "FastBuffer" should start
    };

    return scope.Close(nodeBufCtor->NewInstance(bufArgc, bufArgv));
}

} //namespace xblab

