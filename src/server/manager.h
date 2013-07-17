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
  ~Manager();

  std::map<int, Member> members;
  Group group;    

  std::string publicKey;
  
private:
  std::string privateKey_;
  int nMembers_;
  int currentRound_;
  int *roundModulii_;

};
} //namespace xblab


#endif