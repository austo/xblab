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
  canStartChat(); // not thread-safe

  bool
  allMessagesProcessed(); // not thread-safe


  // TODO: make wcb and awcb members?
  void
  startChatIfNecessary(uv_work_cb wcb /*, uv_after_work_cb awcb*/);

  void
  broadcastIfNecessary(uv_work_cb wcb /*, uv_after_work_cb awcb*/);

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

  // static uv_after_work_cb
  static void
  afterRoundWork(uv_work_t *r); 
  
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