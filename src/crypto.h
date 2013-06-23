#ifndef CRYPTO_H
#define CRYPTO_H

#include <string>
#include <exception>

#include <botan/botan.h>
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
    static bool verify(Botan::RSA_PublicKey* rsakey, std::string message, std::string signature);
    static std::string encrypt(std::string& plaintext);
    static std::string encrypt(std::string& publicKey, std::string& plaintext);
    static std::string decrypt(std::string& privateKey, std::string& ciphertext);
    static std::string sign(std::string& privateKey, std::string& message);
    static std::string hashPassword(std::string& pw);




    #ifndef XBLAB_CLIENT

    static std::string keyPassPhrase();
    static std::string privateKeyFile();
    static std::string sign(std::string message);

    #endif    
};

} //namespace xblab

#endif