#include <cstdlib>
#include <exception>

#include "manager.h"
#include "db.h"
#include "db_exception.h"
#include "common/crypto.h"
#include "common/macros.h"
#include "common/common.h"


using namespace std;

namespace xblab{


template <class T>
void
Manager::cleanMemberSchedules(
  std::vector< std::vector<T>* >& schedules) {
  if (schedules.size() < 2) { return; }

  T i = 0, s = 0, curr;  
  std::vector<T> *firstSched = schedules.at(s);

  for (; i < XBSCHEDULESIZE; i++) {
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
  map<int, Member>::iterator mitr = members.begin();
  for (; mitr != members.end(); ++mitr) {
    mitr->second.manager = this;
    schedules.push_back(&mitr->second.schedule);
  }

  nMembers_ = members.size();
  roundModulii_ = NALLOC(nMembers_, int);
  cleanMemberSchedules(schedules);
  currentRound_ = 0;
  cout << rightnow() << "Manager created for group "
      << group.url << " (\"" << group.name << "\")" << endl;  
}


Manager::~Manager(){
  free(roundModulii_);
}


} // namespace xblab


