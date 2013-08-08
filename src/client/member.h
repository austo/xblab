#ifndef MEMBER_H
#define MEMBER_H

#include <string>
#include <vector>

#include "common/common.h"

namespace xblab {

struct Member {
  Member(){
    hasKeys = false;
    hasMessage = false;
    ready = false;
  };  

  std::string username;
  std::string password;
  std::string handle;
  std::string publicKey;
  std::string privateKey;
  std::string sessionKey; // session public key from server 
  std::vector<sched_t> schedule;
  std::string message;
  sched_t modulo;
  bool hasKeys;
  bool ready;
  bool hasMessage; // must be set to false on msg hand-off

};
} //namespace xblab


#endif