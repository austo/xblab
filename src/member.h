#ifndef MEMBER_H
#define MEMBER_H

#include <iostream>
#include <string>
#include <vector>
#include "crypto.h"

namespace xblab {

// Used by Manager for tracking users once they have joined a chat

struct Member {
    Member(){
        username = "invalid";
    };

    Member(std::string username, std::string password, std::string handle) : 
        username(username), password(password), handle(handle) { }

    Member(std::string username, std::string password, std::string pubkey, bool present) : 
        username(username), password(password), public_key(pubkey), present(present) { }

    void assume(const Member& other){
        // std::cout << "assume: " << other.public_key;
        public_key = other.public_key;
        // round_nonce = other.round_nonce;
        present = other.present;
        // delete other
    }

    bool operator== (const Member& other) const {
        std::cout << "this->username: " << username << "\nother.username: " << other.username << std::endl;
        return username == other.username &&
            Crypto::checkPasshash(password, other.password);        
    }

    std::string username;
    std::string password;
    std::string handle;
    std::string public_key;
    std::string round_nonce;      
    int round_modulus;
    bool present;

};
} //namespace xblab


#endif