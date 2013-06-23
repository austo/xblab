#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <vector>
#include <memory>

#include <node.h>

#include <botan/pubkey.h>
#include <botan/base64.h>
#include <botan/hex.h>
#include <botan/lookup.h>
#include <botan/bcrypt.h>

#include "macros.h"
#include "crypto.h"


using namespace std;
using namespace Botan;

namespace xblab {

extern v8::Persistent<v8::String> pub_key_filename;

string Crypto::publicKeyFile(){
    static string retval = string(*(v8::String::Utf8Value(pub_key_filename)));
    return retval;
}

#ifndef XBLAB_CLIENT //Client doesn't need private keys stored on filesystem

extern v8::Persistent<v8::String> priv_key_filename;
extern v8::Persistent<v8::String> key_passphrase;

string Crypto::privateKeyFile(){
    static string retval = string(*(v8::String::Utf8Value(priv_key_filename)));
    return retval;
}

string Crypto::keyPassPhrase(){
    static string retval = string(*(v8::String::Utf8Value(key_passphrase)));
    return retval;
}


string Crypto::sign(string message){
    AutoSeeded_RNG rng;
    string pr = privateKeyFile();
    string pw = keyPassPhrase();
    auto_ptr<PKCS8_PrivateKey> key(PKCS8::load_key(pr, rng, pw));
    RSA_PrivateKey* rsakey = dynamic_cast<RSA_PrivateKey*>(key.get());

    if(!rsakey){
        cout << "BAD KEY!!" << endl;
        throw crypto_exception("Invalid key");
    }

    return sign(rng, rsakey, message);    
}


#endif


string Crypto::sign(string& privateKey, string& message){
    AutoSeeded_RNG rng;
    DataSource_Memory ds(privateKey);
    auto_ptr<PKCS8_PrivateKey> key(PKCS8::load_key(ds, rng));
    RSA_PrivateKey* rsakey = dynamic_cast<RSA_PrivateKey*>(key.get());

    if(!rsakey){
        cout << "BAD KEY!!" << endl;
        throw crypto_exception("Invalid key");
    }

    return sign(rng, rsakey, message);
}


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


bool Crypto::verify(string message, string signature){
    std::auto_ptr<X509_PublicKey> key(X509::load_key(publicKeyFile()));
    RSA_PublicKey* rsakey = dynamic_cast<RSA_PublicKey*>(key.get());

    if(!rsakey) {
        cout << "BAD KEY!!" << endl;
        throw crypto_exception("Invalid key");
    }
    return verify(rsakey, message, signature);
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


// TODO: refactor/cleanup - look at options for smaller keys (should read into buffer)

string Crypto::encrypt(string& plaintext){

    std::auto_ptr<X509_PublicKey> key(X509::load_key(publicKeyFile()));
    RSA_PublicKey* rsakey = dynamic_cast<RSA_PublicKey*>(key.get());

    cout << "plaintext: " << plaintext << endl;
    cout << "plaintext length: " << plaintext.size() << endl;

    //X509_PublicKey *rsakey = X509::load_key(publicKeyFile());

    cout << "got key\n";


    if(!rsakey) {
        cout << "BAD KEY!!" << endl;
        throw crypto_exception("Invalid key");
    }

    AutoSeeded_RNG rng;

    cout << "after rng\n";

    PK_Encryptor *enc = new PK_Encryptor_EME(*rsakey, SHA256);    

    cout << "after end\n";

    // Pipe dpipe(new Base64_Decoder);
    // dpipe.process_msg(plaintext);
    // SecureVector<byte> pt = dpipe.read_all();

    const unsigned char* data = (const unsigned char *)&plaintext[0];
    cout << "after cast\n";
    

    // TODO: try looping this with smaller chunks
    SecureVector<byte> ciphertext = enc->encrypt(data, plaintext.size(), rng);

    cout << "after enc\n";

    Pipe epipe(new Base64_Encoder);
    epipe.process_msg(ciphertext);

    delete enc;    
    return epipe.read_all_as_string();
}


string Crypto::encrypt(string& publicKey, string& plaintext){
    AutoSeeded_RNG rng;
    DataSource_Memory ds(publicKey);
    X509_PublicKey *rsakey = X509::load_key(ds);
    PK_Encryptor *enc = new PK_Encryptor_EME(*rsakey, EMESHA1);

    cout << "encryptor max input size: " << enc->maximum_input_size() << endl;

    const unsigned char* data = (const unsigned char *)&plaintext[0];
        
    SecureVector<byte> ciphertext = enc->encrypt(data, plaintext.size(), rng);
    Pipe pipe(new Base64_Encoder);
    pipe.process_msg(ciphertext);

    delete enc;    
    return pipe.read_all_as_string();
}

string Crypto::decrypt(string& privateKey, string& ciphertext){
    AutoSeeded_RNG rng;

    DataSource_Memory ds(privateKey);
    PKCS8_PrivateKey *rsakey = PKCS8::load_key(ds, rng);
    PK_Decryptor *dec = new PK_Decryptor_EME(*rsakey, SHA256);

    Pipe pipe(new Base64_Decoder);
    pipe.process_msg(ciphertext);
    SecureVector<byte> cipherbytes = pipe.read_all();
    
    SecureVector<byte> plaintext = dec->decrypt(cipherbytes);

    string retval(plaintext.begin(), plaintext.end());

    delete dec;
    return retval;
}


string Crypto::generateNonce(){
    SecureVector<byte> buf(NONCE_SIZE);
    AutoSeeded_RNG rng;
    rng.randomize(buf, buf.size());
    Pipe pipe(new Base64_Encoder);
    pipe.process_msg(buf);
    return pipe.read_all_as_string();
}


void Crypto::generateKey(string& pr, string& pu){    
    AutoSeeded_RNG rng;
    RSA_PrivateKey key(rng, BITSIZE);
    pr = PKCS8::PEM_encode(key);
    pu = X509::PEM_encode(key);
}


string Crypto::hashPassword(string& pw){
    AutoSeeded_RNG rng;
    return generate_bcrypt(pw, rng, BCRYPT_WORK_FACTOR);
}



//TODO: encrypt/decrypt 

} //namespace xblab