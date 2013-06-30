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

string Util::needCredBuf(string& privKeyFile, string& password, string& nonce){ 
    // cout << "generating nonce\n";   
    nonce = Crypto::generateNonce();
    // cout << "have nonce\n";
    Broadcast bc;
    Broadcast::Data *data = new Broadcast::Data();
    // cout << "have data\n";

    data->set_type(Broadcast::NEEDCRED);
    data->set_nonce(nonce);

    string sig, datastr;
    if (!data->SerializeToString(&datastr)) {
        throw util_exception("Failed to serialize broadcast data.");
    }
    try{
        // cout << "signing message\n";
        // cout << privKeyFile << endl << password << endl;
        sig = Crypto::sign(privKeyFile, password, datastr);
        // cout << "message signed\n";
        bc.set_signature(sig);
        // cout << "signature set\n";
        bc.set_allocated_data(data); //bc now owns data - no need to free
        // cout << "data allocated\n";
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

        // TODO: refactor, change to switch
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

            Manager* mgr;

            // Create manager and add to xblab->Managers collection
            // TODO: refactor into method, maybe template
            if (managers.find(url) == managers.end()){

                HandleScope scope;

                Local<ObjectTemplate> t = ObjectTemplate::New();
                t->SetInternalFieldCount(1);
                Local<Object> holder = t->NewInstance();

                mgr = new Manager(url);                

                mgr->Wrap(holder);
                managers.insert(pair<string, Handle<Object> >(url, mgr->handle_));
                scope.Close(Undefined());           
            }
            else{
                cout << "Unwrapping manager...\n";
                mgr = node::ObjectWrap::Unwrap<Manager>(managers.at(url));
            }

            map<int, Member>::iterator mitr = mgr->members_.begin();
            for (; mitr != mgr->members_.end(); ++mitr){

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
        }
    }
}


void Util::unpackMember(DataBaton* baton){
    string buf = Crypto::hybridDecrypt(baton->privateKeyFile, baton->password, baton->buf);

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

    if (baton->nonce == retNonce){

        // TODO: refactor, change to switch
        if (data.type() == Transmission::CRED){
            const Transmission::Credential& cred = data.credential();
            string pubkey(cred.pub_key());

            if (!Crypto::verify(pubkey, datastr, trans.signature())) { 
                throw util_exception("User key not verified.");
            }

            cout << "Parse transmission: user signature verified.\n";
            string un(cred.username());
            string pw(cred.password());
            baton->url = cred.group();

            baton->auxData = new Member(un, pw, pubkey, true);
        }
    }
    else{
        throw util_exception("Last nonce does not match transmission nonce.");
    }
}


void Util::addMember(DataBaton* baton, map<string, Handle<Object> >& managers){
    Member *member = reinterpret_cast<Member*>(baton->auxData);
    // Create manager and add to xblab->Managers collection
    // TODO: refactor into method, maybe template
    Manager *mgr;

    if (managers.find(baton->url) == managers.end()){

        HandleScope scope;

        Local<ObjectTemplate> t = ObjectTemplate::New();
        t->SetInternalFieldCount(1);
        Local<Object> holder = t->NewInstance();

        mgr = new Manager(baton->url);                

        mgr->Wrap(holder);
        managers.insert(pair<string, Handle<Object> >(baton->url, mgr->handle_));
        scope.Close(Undefined());           
    }
    else{
        cout << "Unwrapping manager...\n";
        mgr = node::ObjectWrap::Unwrap<Manager>(managers.at(baton->url));
    }

    map<int, Member>::iterator mitr = mgr->members_.begin();
    for (; mitr != mgr->members_.end(); ++mitr){

        try {

            if (*member == mitr->second){
                mitr->second.assume(member);
                cout << "member " << mitr->second.username << " assumed with pub_key:\n"
                     << mitr->second.public_key << endl;
            }
        }
        catch (exception& e){
            cout << e.what();
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

    cred->set_username(participant->username_);

    // Plaintext password will be compared with stored passhash on the server
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

    Crypto::hybridEncrypt(plaintext, ciphertext);

    return ciphertext.str();
}

// Mainly for consumption by the client, return Broadcast::Type
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
        slowBuffer->handle_,    // JS SlowBuffer handle
        v8::Integer::New(len),  // SlowBuffer length
        v8::Integer::New(0)     // Offset where "FastBuffer" should start
    };

    return scope.Close(nodeBufCtor->NewInstance(bufArgc, bufArgv));
}

} //namespace xblab

