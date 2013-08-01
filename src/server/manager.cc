#include <cstdlib>
#include <exception>

#include <unistd.h>

#include "manager.h"
#include "db.h"
#include "db_exception.h"
#include "common/crypto.h"
#include "common/macros.h"
#include "common/common.h"


using namespace std;

namespace xblab {

typedef map<int, Member>::iterator memb_iter;

template <class T>
void
Manager::cleanMemberSchedules(
  std::vector< std::vector<T>* >& schedules, size_t elemsize) {
  if (schedules.size() < 2) { return; }

  T i = 0, s = 0, curr;  
  std::vector<T> *firstSched = schedules.at(s);

  for (; i < elemsize; i++) {
    curr = (*firstSched)[i];

  #ifdef DEBUG
    std::cout << "\n(0) " << curr << ", ";
  #endif

    for (s = 1; s < schedules.size(); s++) {

    #ifdef DEBUG
      std::cout << "(" << s << ") " << (*schedules.at(s))[i] << ", ";
    #endif

      if ((*schedules.at(s))[i] == curr) {
        (*schedules.at(s))[i] += 1;
        ++curr;

      #ifdef DEBUG
        std::cout << "\nincremented curr: " << curr << std::endl;
      #endif

      }
    }
  }
}


Manager::Manager(string url) {
  Crypto::generateKey(this->privateKey_, this->publicKey);
  group = Db::getGroup(url);
  
  // We've got the room ID, now get our members
  members = Db::getMembers(group.id);
  seed = Crypto::generateRandomInt<unsigned>();

  vector< vector<sched_t>* > schedules;
  // Each member has a reference to the manager
  memb_iter mitr = members.begin();
  for (; mitr != members.end(); ++mitr) {
    mitr->second.manager = this;
    schedules.push_back(&mitr->second.schedule);
  }

  nMembers_ = members.size();
  roundModulii_ = NALLOC(nMembers_, int);
  cleanMemberSchedules(schedules, XBSCHEDULESIZE);
  currentRound_ = 0;
  chatStarted = false; // yes, these are both unnecessary
  chatStarting = false;
  cout << rightnow() << "Manager created for group "
      << group.url << " (\"" << group.name << "\")" << endl;  
}


Manager::~Manager(){
  free(roundModulii_);
}

bool
Manager::allMembersPresent() {
  memb_iter mitr = members.begin();
  for (; mitr != members.end(); ++mitr) {
    if (!mitr->second.present) {
      return false;
    }
  }
  return true;
}


void
Manager::broadcastStartChat() {
  cout << "notifying " << group.name << endl;
  usleep(50000); // TODO: remove when we've got a plan

  memb_iter mitr = members.begin();
  for (; mitr != members.end(); ++mitr) {
    mitr->second.notifyStartChat();
  }
}


string
Manager::decryptSessionMessage(string& ciphertext) {
  return Crypto::hybridDecrypt(privateKey_, ciphertext);
}


/* static */
// TODO: remove this in favor of manager-by-manager approach
void
Manager::notifyPackedGroups(map<string, Manager*> *mgrs) {
  map<string, Manager*>::iterator mgr_iter = mgrs->begin();
  usleep(10000); // TODO: remove when we've got a plan
  cout << "notifying packed groups...\n";
}


} // namespace xblab


