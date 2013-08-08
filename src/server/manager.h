#ifndef MANAGER_NATIVE_H
#define MANAGER_NATIVE_H

#include <iostream>
#include <string>
#include <vector>
#include <map>

#include <uv.h>

#include "group.h"
#include "member.h"
#include "common/common.h"


namespace xblab {

class Manager {

public:
  Manager(std::string url);    
  ~Manager();

  std::map<int, Member> members;
  Group group;    

  std::string publicKey;

  sched_t
  getTargetModulo();

  bool
  allMembersPresent();

  bool
  allMembersReady();

  void
  getStartChatBuffers();

  void
  broadcast();

  void
  endChat();

  std::string
  getPrivateKey() {
    return privateKey_;
  }

  std::string
  decryptSessionMessage(std::string& ciphertext);  
  
private:
  int currentRound_;
  sched_t targetModulo_;
  std::string privateKey_;
  bool moduloCalculated_;
  bool chatStarted_;

  uv_mutex_t classMutex_;
  uv_mutex_t propertyMutex_;

  template <class T>
  void
  cleanMemberSchedules(
    std::vector< std::vector<T>* >& schedules, size_t n);
};

} //namespace xblab

#endif