#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <memory>

#include <botan/base64.h>
#include <botan/hex.h>
#include <botan/lookup.h>
#include <botan/bcrypt.h>

#include "macros.h"
#include "crypto.h"


using namespace std;
using namespace Botan;

namespace xblab {

extern string xbPublicKeyFile;

#ifndef XBLAB_CLIENT // These methods assume a private key

extern string xbPrivateKeyFile;
extern string xbKeyPassword;

string
Crypto::sign(string& message){
  AutoSeeded_RNG rng;
  // cout << pr << pw << endl;
  auto_ptr<PKCS8_PrivateKey> key(
    PKCS8::load_key(xbPrivateKeyFile, rng, xbKeyPassword));
  RSA_PrivateKey* rsakey = dynamic_cast<RSA_PrivateKey*>(key.get());

  if(!rsakey){
    cout << "BAD KEY!!" << endl;
    throw crypto_exception("Invalid key");
  }

  return sign(rng, rsakey, message);    
}


string
Crypto::hybridDecrypt(string& ciphertext){
  AutoSeeded_RNG rng;
  auto_ptr<PKCS8_PrivateKey> key(
    PKCS8::load_key(xbPrivateKeyFile, rng, xbKeyPassword));
  RSA_PrivateKey* rsakey = dynamic_cast<RSA_PrivateKey*>(key.get());

  if(!rsakey){
    cout << "BAD KEY!!" << endl;
    throw crypto_exception("Invalid key");
  }

  stringstream ctstream(ciphertext);
  stringstream ptstream;
  hybridDecrypt(rng, rsakey, ctstream, ptstream);

  return ptstream.str();
}

#endif


string
Crypto::sign(string& privateKey, string& message){
  AutoSeeded_RNG rng;
  DataSource_Memory ds(privateKey);
  // cout << "got DataSource_Memory\n";
  auto_ptr<PKCS8_PrivateKey> key(PKCS8::load_key(ds, rng));
  // cout << "loaded key\n";
  RSA_PrivateKey* rsakey = dynamic_cast<RSA_PrivateKey*>(key.get());
  // cout << "got key\n";

  if(!rsakey){
    cout << "BAD KEY!!" << endl;
    throw crypto_exception("Invalid key");
  }

  return sign(rng, rsakey, message);
}


string
Crypto::sign(AutoSeeded_RNG& rng, RSA_PrivateKey*& rsakey, string& message){
  PK_Signer signer(*rsakey, SHA1);

  DataSource_Memory in(message);
  byte buf[SIG_BUF_SIZE] = { 0 };
  while(size_t got = in.read(buf, sizeof(buf))){
    signer.update(buf, got);
  }
  stringstream ss;
  ss << base64_encode(signer.signature(rng));
  return ss.str();
}


bool
Crypto::verify(string publicKey, string message, string signature){
  DataSource_Memory ds(publicKey);

  std::auto_ptr<X509_PublicKey> key(X509::load_key(ds));
  RSA_PublicKey* rsakey = dynamic_cast<RSA_PublicKey*>(key.get());

  if(!rsakey) {
    cout << "BAD KEY!!" << endl;
    throw crypto_exception("Invalid key");
  }
  return verify(rsakey, message, signature);
}


bool
Crypto::verify(string message, string signature){
  std::auto_ptr<X509_PublicKey> key(X509::load_key(xbPublicKeyFile));
  RSA_PublicKey* rsakey = dynamic_cast<RSA_PublicKey*>(key.get());

  if(!rsakey) {
    cout << "BAD KEY!!" << endl;
    throw crypto_exception("Invalid key");
  }
  return verify(rsakey, message, signature);
}


bool
Crypto::verify(RSA_PublicKey* rsakey, string message, string signature){   

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
  
  return ok;
}

/*
  hybridEncrypt and hybridDecrypt are adaped from examples
  by Botan author Jack Lloyd, and are hereby distributed
  under the terms of the Botan license.
*/

string
Crypto::hybridEncrypt(string& publicKey, string& plaintext){
  AutoSeeded_RNG rng;
  DataSource_Memory ds(publicKey);

  std::auto_ptr<X509_PublicKey> key(X509::load_key(ds));
  RSA_PublicKey* rsakey = dynamic_cast<RSA_PublicKey*>(key.get());

  stringstream ptstream(plaintext), ctstream;

  hybridEncrypt(rsakey, ptstream, ctstream);
  return ctstream.str();
}


void
Crypto::hybridEncrypt(stringstream& in, stringstream& out){        

  std::auto_ptr<X509_PublicKey> key(X509::load_key(xbPublicKeyFile));
  RSA_PublicKey* rsakey = dynamic_cast<RSA_PublicKey*>(key.get());

  if(!rsakey) {
    throw crypto_exception("Invalid key");
  }

  hybridEncrypt(rsakey, in, out);
}


void
Crypto::hybridEncrypt(
  RSA_PublicKey* rsakey, stringstream& in, stringstream& out){
  try {

    AutoSeeded_RNG rng;
    PK_Encryptor_EME encryptor(*rsakey, EMESHA1);

    /*
      Generate the master key from which others are derived.

      Make the master key as large as can be encrypted by the public key,
      up to a limit of 256 bits. For 512-bit public keys, 
      he master key will be >160 bits.
      A >600 bit public key will use the full 256-bit master key.

      In theory, this is not enough, because
      including the initial vector, we derive 320 bits
      (16 + 16 + 8 = 40 bytes) of secrets using the master key.
      In practice, however, it should be fine.
    */

    SymmetricKey masterkey(
      rng, std::min<size_t>(32, encryptor.maximum_input_size()));

    SymmetricKey cast_key = deriveSymmetricKey(CAST, masterkey, CASTBYTES);
    SymmetricKey hmac_key = deriveSymmetricKey(MAC, masterkey, MACBYTES);
    SymmetricKey init_vec = deriveSymmetricKey(IV, masterkey, IVBYTES);

    SecureVector<byte> encrypted_key =
      encryptor.encrypt(masterkey.bits_of(), rng);

    out << b64Encode(encrypted_key) << endl;

    Pipe pipe(
      new Fork(
        new Chain(
          get_cipher(CAST128, cast_key, init_vec, ENCRYPTION),
          new Base64_Encoder(true)),
        new Chain(
          new MAC_Filter(HMACSHA1, hmac_key, MACOUTLEN),
          new Base64_Encoder)
      )
    );

    pipe.start_msg();
    in >> pipe;
    pipe.end_msg();

    /* 
      Write the MAC as the second line. That way we can pull it off right
      from the start and feed the rest of the file right into a pipe on the
      decrypting end.
    */

    out << pipe.read_all_as_string(1) << endl;
    out << pipe.read_all_as_string(0);

  }
  catch(exception& e) {
    cout << "Exception: " << e.what() << endl;
  }
}



string
Crypto::hybridDecrypt(string& privateKey, string& ciphertext){
  AutoSeeded_RNG rng;
  DataSource_Memory ds(privateKey);
  auto_ptr<PKCS8_PrivateKey> key(PKCS8::load_key(ds, rng));
  RSA_PrivateKey* rsakey = dynamic_cast<RSA_PrivateKey*>(key.get());

  if(!rsakey){
    cout << "BAD KEY!!" << endl;
    throw crypto_exception("Invalid key");
  }

  // cout << "after got key\n";
  stringstream ctstream(ciphertext);
  stringstream ptstream;
  hybridDecrypt(rng, rsakey, ctstream, ptstream);

  return ptstream.str();
}


void
Crypto::hybridDecrypt(AutoSeeded_RNG& rng,
  RSA_PrivateKey*& rsakey, stringstream& in, stringstream& out){
  try {      

    string encryptingMasterKeyString;
    getline(in, encryptingMasterKeyString);

    cout << "encryptingMasterKeyString:\n" 
         << encryptingMasterKeyString << endl;

    string macString;
    getline(in, macString);

    cout << "macString:\n" << macString << endl;

    SecureVector<byte> encryptingMasterKey =
      b64Decode(encryptingMasterKeyString);

    PK_Decryptor_EME decryptor(*rsakey, EMESHA1);

    SecureVector<byte> masterkey = decryptor.decrypt(encryptingMasterKey);

    SymmetricKey cast_key = deriveSymmetricKey(CAST, masterkey, CASTBYTES);
    SymmetricKey hmac_key = deriveSymmetricKey(MAC, masterkey, MACBYTES);
    InitializationVector init_vec = deriveSymmetricKey(IV, masterkey, IVBYTES);


    Pipe pipe(
      new Base64_Decoder, get_cipher(CAST128, cast_key, init_vec, DECRYPTION),
      new Fork(
        0, new Chain(
          new MAC_Filter(HMACSHA1, hmac_key, MACOUTLEN),
          new Base64_Encoder)
      )
    );

    pipe.start_msg();
    in >> pipe;
    pipe.end_msg();

    string targetMac = pipe.read_all_as_string(1);

    if(targetMac != macString) {
      cout << "WARNING: MAC in message failed to verify\n";
    }

    out << pipe.read_all_as_string(0);

   }
   catch(exception& e) {
    cout << "Exception caught: " << e.what() << endl;
  }
}


string
Crypto::generateNonce(){
  SecureVector<byte> buf(NONCE_SIZE);
  AutoSeeded_RNG rng;
  rng.randomize(buf, buf.size());
  Pipe pipe(new Base64_Encoder);
  pipe.process_msg(buf);
  return pipe.read_all_as_string();
}

int
Crypto::generateRandomInt(){
  SecureVector<byte> buf(sizeof(int));
  AutoSeeded_RNG rng;
  rng.randomize(buf, buf.size());
  int random = int((int)buf[0]);
  return random;
}


void
Crypto::generateKey(string& pr, string& pu){    
  AutoSeeded_RNG rng;
  RSA_PrivateKey key(rng, BITSIZE);
  pr = PKCS8::PEM_encode(key);
  pu = X509::PEM_encode(key);
}


string
Crypto::hashPassword(string& pw){
  AutoSeeded_RNG rng;
  return generate_bcrypt(pw, rng, BCRYPT_WORK_FACTOR);
}

bool
Crypto::checkPasshash(string pw, string ph){
  cout << "password: " << pw << endl << "passhash: " << ph << endl;
  cout << "passhash length: " << ph.size() << endl;

  return ph.size() == BCRYPT_PH_SIZE && check_bcrypt(pw, ph);
}


string
Crypto::b64Encode(const SecureVector<byte>& in) {
  Pipe pipe(new Base64_Encoder);
  pipe.process_msg(in);
  return pipe.read_all_as_string();
}


Botan::SecureVector<byte>
Crypto::b64Decode(const std::string& in) {
  Pipe pipe(new Base64_Decoder);
  pipe.process_msg(in);
  return pipe.read_all();
}


Botan::SymmetricKey
Crypto::deriveSymmetricKey(const std::string& param,
  const SymmetricKey& masterkey, u32bit outputlength) {

  std::auto_ptr<KDF> kdf(get_kdf(KDF2SHA1));
  return kdf->derive_key(outputlength, masterkey.bits_of(), param);
}

int
Crypto::init(){
  try {
    // Start crypto on module load
    Botan::LibraryInitializer init("thread_safe=true");
    return 0;
  }
  catch(std::exception& e) {
    std::cerr << e.what() << "\n";
    return 1;
  }
}

} //namespace xblab