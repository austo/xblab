#include <iostream>
#include <sstream>
#include <vector>
#include <map>

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
#include "manager.h"
#include "member.h"
#include "db.h"


using namespace std;
using namespace Botan;
using namespace v8;


#define DL_EX_PREFIX "Util: "


namespace xblab {

extern v8::Persistent<v8::Function> nodeBufCtor;

Util::Util(){ /* The goal is to keep this a "static" class */ }
Util::~Util(){}

#ifndef XBLAB_CLIENT

string Util::needCredBuf(string& nonce){    
    nonce = Crypto::generateNonce();
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


// TODO: error handling
// Mainly for server use
void Util::parseTransmission(string lastNonce,
    string& ciphertext, map<string, Handle<Object> >& managers){

    string buf = Crypto::hybridDecrypt(ciphertext);

    //TODO: switch on transmission type
    Transmission trans;
    if (!trans.ParseFromString(buf)){
        throw util_exception("Failed to deserialize transmission.");
    }    

    string datastr;
    if (!trans.data().SerializeToString(&datastr)){
        throw util_exception("Failed to reserialize transmission data.");
    }

    const Transmission::Data& data = trans.data();

    string retNonce(data.return_nonce());
    // cout << "incoming return nonce: " << retNonce << endl;
    // cout << "saved return nonce: " << lastNonce << endl;

    if (lastNonce == retNonce){

        // TODO: switch
        if (data.type() == Transmission::CRED){
            const Transmission::Credential& cred = data.credential();
            string pubkey(cred.pub_key());

            if (!Crypto::verify(pubkey, datastr, trans.signature())) { 
                return;
            }

            cout << "Parse transmission: user signature verified.\n";
            string un(cred.username());
            string pw(cred.password());
            string url(cred.group());

            Member testMember = Member(un, pw, pubkey, true);

            cout << "testMember created for " << un << endl;

            // Create manager and add to xblab->Managers collection
            if (managers.find(url) == managers.end()){

                HandleScope scope;

                Local<ObjectTemplate> t = ObjectTemplate::New();
                t->SetInternalFieldCount(1);
                Local<Object> holder = t->NewInstance();

                Manager* mgrInstance = new Manager(url);

                map<int, Member>::iterator mitr = mgrInstance->members_.begin();
                for (; mitr != mgrInstance->members_.end(); ++mitr){
                    cout << "Member: " << mitr->second.username << endl;

                    try {

                        if (testMember == mitr->second){
                            mitr->second.assume(testMember);
                            cout << "member " << mitr->second.username << " assumed with pub_key:\n"
                                 << mitr->second.public_key << endl;
                        }
                    }
                    catch (exception& e){
                        cout << e.what();
                    }
                }

                mgrInstance->Wrap(holder);
                managers.insert(pair<string, Handle<Object> >(url, mgrInstance->handle_));
                scope.Close(Undefined());
           
            }
            else {
                cout << "manager already created." << endl;

            }
        }
    }
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

    // string passhash = Crypto::hashPassword(participant->password_);
    // cout << "passhash: " << passhash << endl;

    cred->set_username(participant->username_);
    cred->set_password(participant->password_);
    cred->set_group(participant->group_);
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

    string datastr;
    if (!bc.data().SerializeToString(&datastr)){
        throw util_exception("Failed to reserialize broadcast data.");
    }

    if (Crypto::verify(datastr, bc.signature())){
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

