#ifndef CRYPTO_H
#define CRYPTO_H

#include <string>
#include <sstream>
#include <exception>

#include <botan/botan.h>
#include <botan/pubkey.h>
#include <botan/rsa.h>


namespace xblab {

class crypto_exception : public std::exception {
public:
    crypto_exception(){
        message_ = std::string("xblab crypto exception");
    }
    crypto_exception(std::string err_msg){
        message_ = err_msg;
    }
    ~crypto_exception() throw(){};
    virtual const char* what() const throw(){
        return this->message_.c_str();
    }
private:
    std::string message_;        
};

class Crypto {

public:
    
    static std::string publicKeyFile();
    static void generateKey(std::string& pr, std::string& pu);
    static std::string generateNonce();    

    static std::string sign(Botan::AutoSeeded_RNG&, Botan::RSA_PrivateKey*&, std::string&);

    static bool verify(std::string message, std::string signature);
    static bool verify(std::string publicKey, std::string message, std::string signature);
    static bool verify(Botan::RSA_PublicKey* rsakey, std::string message, std::string signature);

    static void hybridEncrypt(std::stringstream& in, std::stringstream& out);
    static std::string hybridEncrypt(std::string& publicKey, std::string& plaintext);

    static std::string hybridDecrypt(std::string& privateKey, std::string& ciphertext);

    
    static std::string sign(std::string& privateKey, std::string& message);
    static std::string hashPassword(std::string& pw);
    static bool checkPasshash(std::string pw, std::string ph);





    #ifndef XBLAB_CLIENT

    static std::string keyPassPhrase();
    static std::string privateKeyFile();
    static std::string sign(std::string message);
    static std::string sign(std::string& privateKey, std::string& password, std::string& message);


    static std::string hybridDecrypt(std::string& ciphertext);
    static std::string
    hybridDecrypt(std::string& privateKey, std::string& password, std::string& ciphertext);


    #endif

private:

    // Convenience wrappers for Botan Pipe functions
    static std::string b64Encode(const Botan::SecureVector<unsigned char>&);
    static Botan::SecureVector<unsigned char> b64Decode(const std::string& in);
    static Botan::SymmetricKey deriveSymmetricKey(const std::string&, const Botan::SymmetricKey&, unsigned int);
    static void hybridEncrypt(Botan::RSA_PublicKey* rsakey, std::stringstream& in, std::stringstream& out);

    static void hybridDecrypt(Botan::AutoSeeded_RNG&,
        Botan::RSA_PrivateKey*&, std::stringstream&, std::stringstream&);
};

} //namespace xblab

#endif