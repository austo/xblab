#ifndef PARTICIPANT_H
#define PARTICIPANT_H

#include <string>

namespace xblab {

struct Participant {
  Participant(){
    hasKeys = false;
  };  

  std::string username;
  std::string password;
  std::string handle;
  std::string publicKey;
  std::string privateKey
  int seed;      
  int modulus;
  bool hasKeys;

};
} //namespace xblab


#endif