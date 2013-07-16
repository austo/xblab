#ifndef MEMBER_H
#define MEMBER_H

#include <string>

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
  int seed;      
  int modulus;
  bool hasKeys;

};
} //namespace xblab


#endif