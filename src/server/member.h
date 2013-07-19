#ifndef MEMBER_H
#define MEMBER_H

#include <iostream>
#include <string>
#include <vector>
#include "common/crypto.h"

namespace xblab {

// Used by Manager for tracking users once they have joined a chat

class Manager; // Forward declaration

class Member {
public:
  Member(){
    username = "invalid";
  };

  Member(
    std::string username,
    std::string password,
    std::string handle) :
    username(username),
    password(password),
    handle(handle) { }

  Member(
    std::string username,
    std::string password,
    std::string pubkey,
    bool present) : 
    username(username),
    password(password),
    publicKey(pubkey),
    present(present) { }

  void assume(const Member& other){
    publicKey = other.publicKey;
    present = other.present;
  }

  void assume(Member* other){
    publicKey = other->publicKey;
    present = other->present;
    delete other;
  } 

  bool operator== (const Member& other) const {
    #ifdef DEBUG
    std::cout << "this->username: " << username
          << "\nother.username: " << other.username << std::endl;
    #endif
    return username == other.username &&
      Crypto::checkPasshash(password, other.password);        
  }

  std::string username;
  std::string password;
  std::string handle;
  std::string publicKey;
  std::string roundNonce;
  int roundModulus;
  bool present;
  Manager *manager;

};
} //namespace xblab


#endif