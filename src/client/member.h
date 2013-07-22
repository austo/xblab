#ifndef MEMBER_H
#define MEMBER_H

#include <string>
#include <vector>

#include "common/common.h"

namespace xblab {

struct Member {
  Member(){
    hasKeys = false;
  };  

  std::string username;
  std::string password;
  std::string handle;
  std::string publicKey;
  std::string privateKey;
  std::string sessionKey; // session public key from server 
  std::vector<sched_t> schedule;   
  int modulus;
  bool hasKeys;

};
} //namespace xblab


#endif