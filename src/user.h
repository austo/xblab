#ifndef USER_H
#define USER_H

#include <string>
#include <map>

#include "group.h"

namespace xblab {

// Used by xblab.server for tracking users
// who have authenticated but not joined any chat

struct User {
    User(){};
    User(std::string username, std::string password) : 
        username(username), password(password) { }
    int id;
    std::string username;
    std::string password;
    std::string public_key;
    std::string last_nonce;      
    std::map<int, Group> groups;
};
} //namespace xblab


#endif