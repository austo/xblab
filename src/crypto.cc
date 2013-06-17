#include <node.h>
#include "macros.h"
#include "crypto.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <vector>
#include <memory>
#include <botan/pubkey.h>
#include <botan/base64.h>
#include <botan/hex.h>
#include <botan/lookup.h>

using namespace std;
using namespace Botan;

namespace xblab {

#ifndef XBLAB_CLIENT //Client has no need for keys stored on filesystem

extern v8::Persistent<v8::String> pub_key_filename;
extern v8::Persistent<v8::String> priv_key_filename;
extern v8::Persistent<v8::String> key_passphrase;


string Crypto::pub_key_file(){
    static string retval = string(*(v8::String::Utf8Value(pub_key_filename->ToString())));
    return retval;
}

string Crypto::priv_key_file(){
    static string retval = string(*(v8::String::Utf8Value(priv_key_filename->ToString())));
    return retval;
}

string Crypto::key_password(){
    static string retval = string(*(v8::String::Utf8Value(key_passphrase->ToString())));
    return retval;
}


string Crypto::sign(string message){
    AutoSeeded_RNG rng;
    string pr = priv_key_file();
    string pw = key_password();
    auto_ptr<PKCS8_PrivateKey> key(PKCS8::load_key(pr, rng, pw));
    RSA_PrivateKey* rsakey = dynamic_cast<RSA_PrivateKey*>(key.get());

    if(!rsakey){
        cout << "BAD KEY!!" << endl;
        throw crypto_exception("Invalid key");
    }

    return sign(rng, rsakey, message);    
}


bool Crypto::verify(string message, string signature){
    std::auto_ptr<X509_PublicKey> key(X509::load_key(pub_key_file()));
    RSA_PublicKey* rsakey = dynamic_cast<RSA_PublicKey*>(key.get());

    if(!rsakey) {
        cout << "BAD KEY!!" << endl;
        throw crypto_exception("Invalid key");
    }
    return verify(rsakey, message, signature);
}


#endif


string Crypto::sign(AutoSeeded_RNG& rng, RSA_PrivateKey*& rsakey, string& message){
    PK_Signer signer(*rsakey, SHA1);

    //cout << "message: " << message << endl;

    DataSource_Memory in(message);
    //cout << "crypto::sign message: " << message << endl;
    byte buf[SIG_BUF_SIZE] = { 0 };
    while(size_t got = in.read(buf, sizeof(buf))){
        signer.update(buf, got);
    }
    stringstream ss;
    ss << base64_encode(signer.signature(rng));
    return ss.str();
}


bool Crypto::verify(RSA_PublicKey* rsakey, string message, string signature){   

    Pipe pipe(new Base64_Decoder);
    pipe.process_msg(signature);
    SecureVector<byte> sig = pipe.read_all();

    PK_Verifier ver(*rsakey, SHA1);

    DataSource_Memory in(message);
    byte buf[SIG_BUF_SIZE] = { 0 };
    while(size_t got = in.read(buf, sizeof(buf))){
        ver.update(buf, got);
    }

    const bool ok = ver.check_signature(sig);

    if(ok){
        cout << "Signature verified\n";
    }
    else{         
        cout << "Signature did NOT verify\n";
    }
    return ok;
}


string Crypto::generate_nonce(){
    SecureVector<byte> buf(NONCE_SIZE);
    AutoSeeded_RNG rng;
    rng.randomize(buf, buf.size());
    Pipe pipe(new Base64_Encoder);
    pipe.process_msg(buf);
    return pipe.read_all_as_string();
}


void Crypto::generate_key(string& pr, string& pu){    
    AutoSeeded_RNG rng;
    RSA_PrivateKey key(rng, BITSIZE);
    pr = PKCS8::PEM_encode(key);
    pu = X509::PEM_encode(key);
}

//TODO: encrypt/decrypt 

} //namespace xblab