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

// fwd declaration(s)
class Manager;
class MemberBaton; // access baton through xbManagers

class Member {

public:
  Member(){
    username = "invalid";
    ready = false;
    present = false;
    messageProcessed = false;
    clientHasSchedule = false;
  };


/* NOTE: since groups are defined ahead of time, it *should*
 * be okay to allocate schedules on member initialization,
 * then populate them in the manager constructor. However,
 * this could get out of hand if we start allowing arbitrary
 * members into the group at arbitrary times.
 */
  Member(
    std::string username,
    std::string password,
    std::string handle) :
    username(username),
    password(password),
    handle(handle),    
    schedule(XBSCHEDULESIZE) {
      ready = false;
      present = false;
      messageProcessed = false;
      clientHasSchedule = false;
    }

  Member(
    std::string username,
    std::string password,
    std::string pubkey,
    bool present) : 
    username(username),
    password(password),
    publicKey(pubkey),    
    present(present) {
      ready = false;
      messageProcessed = false;
      clientHasSchedule = false;
    }

  void
  assume(const Member& other);

  void
  assume(Member* other);

  void
  notifyStartChat();

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
  std::vector<sched_t> schedule;
  bool present;
  bool ready;
  bool messageProcessed;
  bool clientHasSchedule; // TODO: better place for this?
  Manager *manager;
  MemberBaton *baton;

};


} //namespace xblab
#endif