#ifndef MANAGER_NATIVE_H
#define MANAGER_NATIVE_H

#include <iostream>
#include <string>
#include <vector>
#include <map>

#include <uv.h>

#include "group.h"
#include "member.h"


namespace xblab {

class Manager {

public:
  Manager(std::string url);    
  ~Manager();

  std::map<int, Member> members;
  Group group;    

  std::string publicKey;

  unsigned seed;

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
  int nMembers_;
  int currentRound_;
  int *roundModulii_;
  std::string privateKey_;
  bool chatStarted_;

  uv_mutex_t mutex_;

  template <class T>
  void
  cleanMemberSchedules(
    std::vector< std::vector<T>* >& schedules, size_t elemsize);
};

} //namespace xblab

#endif