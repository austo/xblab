#ifndef MANAGER_NATIVE_H
#define MANAGER_NATIVE_H

#include <string>
#include <map>

#include "group.h"
#include "member.h"


namespace xblab {
namespace native {

struct Manager {

// public:
    // friend class Util;    

    Manager(std::string url);    
    ~Manager(){};

// private:
    Group group_;
    std::map<int, Member> members_;
    std::string pub_key_;
    std::string priv_key_;
};
} //namespace native
} //namespace xblab


#endif