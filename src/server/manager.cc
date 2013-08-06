#include <cstdlib>
#include <cstdio>

#include <exception>

#include <unistd.h>

#include "manager.h"
#include "memberBaton.h"
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

  if (uv_mutex_init(&mutex_) != XBGOOD) {
    fprintf(stderr, "Error initializing MemberBaton mutex\n");
    throw runtime_error("Error initializing MemberBaton mutex\n");
  }

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
  chatStarted_ = false;
  cout << rightnow() << "Manager created for group "
      << group.url << " (\"" << group.name << "\")" << endl;  
}


Manager::~Manager(){
  free(roundModulii_);
  uv_mutex_destroy(&mutex_);
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

// TODO: make thread-safe (implies members will have to be locked)
// anytime a property is modified
bool
Manager::allMembersReady() {
  // uv_mutex_lock(&mutex_); // TODO: change to rw_lock

  memb_iter mitr = members.begin();
  for (; mitr != members.end(); ++mitr) {
    if (!mitr->second.present || !mitr->second.ready) {
      // uv_mutex_unlock(&mutex_);
      return false;
    }
  }
  // uv_mutex_unlock(&mutex_);
  return true;
}

/* NOTE: we can't call this from an after_work_cb
 *
 * Look into doing this from a uv_idle_t, which we initialize on
 * construction. Once the message has been sent to every member
 * (using uv_queue_work) we can stop the idler
 */
void
Manager::getStartChatBuffers() {
  cout << "inside getStartChatBuffers\n";
  cout << "for " << group.name << endl;
  uv_mutex_lock(&mutex_);

  if (!chatStarted_) {
    cout << "notifying " << group.name << endl;

    memb_iter mitr = members.begin();
    for (; mitr != members.end(); ++mitr) {
      mitr->second.baton->getStartChat();            
    }
    chatStarted_ = true;
  }    

  uv_mutex_unlock(&mutex_);
}


void
Manager::broadcast() {

    uv_mutex_lock(&mutex_);

    cout << "broadcasting " << group.name << endl;

    memb_iter mitr = members.begin();
    for (; mitr != members.end(); ++mitr) {
      mitr->second.baton->unicast();            
    }

    uv_mutex_unlock(&mutex_);
}

// TODO: this method is currently unusable...
string
Manager::decryptSessionMessage(string& ciphertext) {
  string r = Crypto::hybridDecrypt(privateKey_, ciphertext);
  return r;
}


} // namespace xblab

