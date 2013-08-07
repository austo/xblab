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
  std::vector< std::vector<T>* >& schedules, size_t n) {
  if (schedules.size() < 2) { return; }

  T i = 0, s = 0, curr;  
  std::vector<T> *firstSched = schedules.at(s);

  for (; i < n; i++) {
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


bool
Manager::allMembersReady() {

  memb_iter mitr = members.begin();
  for (; mitr != members.end(); ++mitr) {
    if (!mitr->second.present || !mitr->second.ready) {
      return false;
    }
  }
  return true;
}


/* NOTE: if using uv_queue_work, must be called 
 * from uv_work_cb, not uv_after_work_cb
 *
 * Look into doing this from a uv_idle_t, which we initialize on
 * construction. Once the message has been sent to every member
 * (using uv_queue_work) we can stop the idler
 */
void
Manager::getStartChatBuffers() {
#ifdef TRACE
  cout << "inside getStartChatBuffers\n";
  cout << "for " << group.name << endl;
#endif
  uv_mutex_lock(&mutex_);

  if (!chatStarted_) {
    cout << rightnow() << 
      "getting start chat buffers for " << group.name << endl;

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

    cout << rightnow() << "broadcasting start chat to " << group.name << endl;

    memb_iter mitr = members.begin();
    for (; mitr != members.end(); ++mitr) {
      mitr->second.baton->unicast();            
    }

    uv_mutex_unlock(&mutex_);
}


void
Manager::endChat() {
  // TODO: may want to handle member.present 
  // and member.ready here instead of in baton destructor
  if (chatStarted_) {
    uv_mutex_lock(&mutex_);
    chatStarted_ = false;
    uv_mutex_unlock(&mutex_);
  }
}


string
Manager::decryptSessionMessage(string& ciphertext) {
  string r = Crypto::hybridDecrypt(privateKey_, ciphertext);
  return r;
}


} // namespace xblab

