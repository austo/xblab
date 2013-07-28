#ifndef MEMBER_H
#define MEMBER_H

#include <iostream>
#include <string>
#include <vector>

#include "common/crypto.h"
#include "common/common.h"
#include "common/macros.h"

/* Used by Manager to track users once they have joined a chat */

namespace xblab {

// fwd declarations
class Manager;
class MemberBaton; // enables access to baton through managers map


class Member {

public:
  Member(){
    username = "invalid";
    baton = NULL;   
  };


/* NOTE: since groups are defined ahead of time, it *should*
 * be okay to calculate schedules on member initialization, then finish
 * cleaning them in the manager constructor. However,
 * this could get out of hand if we start allowing arbitrary
 * members into the group at arbitrary times.
 */
  Member(
    std::string username,
    std::string password,
    std::string handle) :
    username(username),
    password(password),
    handle(handle) {
      schedule =
        Crypto::generateRandomInts<sched_t>(XBSCHEDULESIZE);
  }


  Member(
    std::string username,
    std::string password,
    std::string pubkey,
    bool present) : 
    username(username),
    password(password),
    publicKey(pubkey),
    present(present) { }


  void
  assume(const Member& other) {
    publicKey = other.publicKey;
    present = other.present;
  }


  void
  assume(Member* other) {
    publicKey = other->publicKey;
    present = other->present;
    delete other;
  }


  bool
  operator== (const Member& other) const {
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
  std::vector<sched_t> schedule;
  bool present;
  Manager *manager;
  MemberBaton *baton;

};


} //namespace xblab
#endif