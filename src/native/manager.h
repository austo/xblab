#ifndef MANAGER_NATIVE_H
#define MANAGER_NATIVE_H

#include <string>
#include <map>

#include "group.h"
#include "member.h"


namespace xblab {

class Manager {

public:
    Manager(std::string url);    
    ~Manager(){};

    std::map<int, Member> members;


private:
    Group group_;
    std::string pub_key_;
    std::string priv_key_;
};
} //namespace xblab


#endif