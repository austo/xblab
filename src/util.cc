#include <node_buffer.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <botan/botan.h>
#include <botan/bcrypt.h>
#include <botan/rsa.h>
#include <botan/pubkey.h>
#include <botan/look_pk.h>
#include "macros.h"
#include "util.h"
#include "crypto.h"
#include "protobuf/xblab.pb.h"


using namespace std;
using namespace Botan;


#define DL_EX_PREFIX "Util: "


namespace xblab {

extern v8::Persistent<v8::Function> node_buf_ctor;

Util::Util(){ /* The goal is to keep this a "static" class */ }
Util::~Util(){}

#ifndef XBLAB_CLIENT

string Util::NeedCredBuf(){    
    string nonce = Crypto::generate_nonce();
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

string Util::ParseBuf(string in){

    //TODO: switch on broadcast type
    Broadcast bc;
    if (!bc.ParseFromString(in)){
        throw util_exception("Failed to deserialize broadcast.");
    }    

    string tst;
    if (!bc.data().SerializeToString(&tst)){
        throw util_exception("Failed to reserialize data.");
    }

    if (Crypto::verify(tst, bc.signature())){
        cout << "Hooray!\n";
    }    

    return bc.DebugString();
}

v8::Local<v8::Value> Util::WrapBuf(const char *c, size_t len){
    v8::HandleScope scope;
    static const unsigned buf_argc = 3;

    node::Buffer *slowBuffer = node::Buffer::New(len);        
    memcpy(node::Buffer::Data(slowBuffer), c, len); // Buffer::Data = (void *)    
   
    v8::Handle<v8::Value> buf_argv[buf_argc] = { 
        slowBuffer->handle_, // JS SlowBuffer handle
        v8::Integer::New(len), // SlowBuffer length
        v8::Integer::New(0) // Offset where "FastBuffer" should start
    };

    return scope.Close(node_buf_ctor->NewInstance(buf_argc, buf_argv));
}

} //namespace xblab

