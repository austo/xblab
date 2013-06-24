#ifndef MEMBER_H
#define MEMBER_H

#include <string>
#include <vector>

namespace xblab {

// Used by Manager for tracking users once they have joined a chat

struct Member {
    Member(){};
    Member(std::string username, std::string password, std::string handle) : 
        username(username), password(password), handle(handle) { }
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