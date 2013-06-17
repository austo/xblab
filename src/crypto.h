#ifndef __CRYPTO_H
#define __CRYPTO_H

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
    
    static std::string generate_nonce();
    static std::string sign(Botan::AutoSeeded_RNG&, Botan::RSA_PrivateKey*&, std::string&);
    static std::string pub_key_file();


    #ifndef XBLAB_CLIENT

    static std::string key_password();
    static std::string priv_key_file();
    static std::string sign(std::string message);

    #endif

    static bool verify(std::string message, std::string signature);
    static bool verify(Botan::RSA_PublicKey* rsakey, std::string message, std::string signature);
    static void generate_key(std::string& pr, std::string& pu);
};

} //namespace xblab

#endif