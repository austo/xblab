#include <iostream>
#include <sstream>
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

Util::Util(){ /* The goal is to keep this a "static" class */ }
Util::~Util(){}

#ifndef XBLAB_CLIENT

string Util::get_need_cred_buf(){
    string nonce = Crypto::generate_nonce();
    Broadcast bc;
    Broadcast::Data *data = new Broadcast::Data();

    data->set_type(Broadcast::NEEDCRED);
    data->set_nonce(nonce);
    cout << "nonce: " << data->nonce() << endl;

    string sig, datastr;
    if (!data->SerializeToString(&datastr)) {
        throw util_exception("Failed to serialize broadcast data.");
    }
    try{
        sig = Crypto::sign(datastr);
        //cout << "sig: " << sig << endl;
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

string Util::parse_buf(string in){

    //TODO: switch on broadcast type
    Broadcast bc;
    if (!bc.ParseFromString(in)){
        throw util_exception("Failed to deserialize broadcast.");
    }

    stringstream ss;
    ss << "signature: " << bc.signature() << endl
         << "data.nonce: " << bc.data().nonce() << endl
         << "data.type: " << bc.data().type() << endl;

    string tst;
    if (!bc.data().SerializeToString(&tst)){
        throw util_exception("Failed to reserialize data.");
    }

#ifndef XBLAB_CLIENT //TODO: no ifdefs in methods!!

    if (Crypto::verify(tst, bc.signature())){
        cout << "Hooray!\n";
    }
    
#endif

    return ss.str();
}

} //namespace xblab

