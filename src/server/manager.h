#ifndef MANAGER_NATIVE_H
#define MANAGER_NATIVE_H

#include <iostream>
#include <string>
#include <vector>
#include <map>

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

  bool chatStarting;
  bool chatStarted;

  bool
  allMembersPresent();

  // TODO: move to group (and add group.cc)?
  static void
  notifyPackedGroups(std::map<std::string, Manager*> *mgrs);
  
private:
  std::string privateKey_;
  int nMembers_;
  int currentRound_;
  int *roundModulii_;

  template <class T>
  void
  cleanMemberSchedules(std::vector< std::vector<T>* >& schedules);


};
} //namespace xblab


#endif