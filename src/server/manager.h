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
  Group group;    

  std::string pub_key;
  
private:
  std::string priv_key_;
};
} //namespace xblab


#endif