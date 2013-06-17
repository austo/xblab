#ifndef __MEMBER_H
#define __MEMBER_H

#include <string>

namespace xblab {

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
};
} //namespace xblab


#endif