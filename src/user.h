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
    // User(int _id, std::string _username, std::string _password) : 
    //     id(_id), username(_username), password(_password) { }

    User(int _id, std::string _username, std::string _password){
        id = _id;
        username = std::string(_username);
        password = std::string(_password);
        groups = std::map<int, Group>();
    }
    int id;
    std::string username;
    std::string password;
    std::string public_key;
    std::string last_nonce;      
    std::map<int, Group> groups;
};
} //namespace xblab


#endif