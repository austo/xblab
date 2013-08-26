#ifndef MEMBER_H
#define MEMBER_H

#include <string>
#include <vector>

#include "common/common.h"
#include "common/crypto.h"

namespace xblab {

struct Member {
  Member() {
    hasMessage = false;
    ready = false;
    currentRound = 0;
    Crypto::generateKey(privateKey, publicKey);
  }
  

  std::string username;
  std::string password;
  std::string handle;
  std::string publicKey;
  std::string privateKey;
  std::string sessionKey; // session public key from server 
  std::vector<sched_t> schedule;
  std::string message;
  sched_t modulo;
  unsigned currentRound;
  bool ready;
  bool hasMessage; // must be set to false on msg hand-off

  bool
  canTransmit() {
    return schedule[currentRound] == modulo;
  }

};
} //namespace xblab


#endif