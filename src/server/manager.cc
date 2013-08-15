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


using namespace std;

namespace xblab {

extern uv_loop_t *loop;

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

  if (uv_mutex_init(&classMutex_) != XBGOOD) {
    fprintf(stderr, "Error initializing MemberBaton mutex\n");
    throw runtime_error("Error initializing MemberBaton mutex\n");
  }

  if (uv_mutex_init(&propertyMutex_) != XBGOOD) {
    fprintf(stderr, "Error initializing MemberBaton mutex\n");
    throw runtime_error("Error initializing MemberBaton mutex\n");
  }

  Crypto::generateKey(this->privateKey_, this->publicKey);
  group = Db::getGroup(url);
  
  // We've got the room ID, now get our members
  members = Db::getMembers(group.id);  

  memb_iter mitr = members.begin();
  for (; mitr != members.end(); ++mitr) {
    mitr->second.manager = this;
  }

  currentRound_ = 0;
  chatStarted_ = false;
  moduloCalculated_ = false;
  schedulesDelivered_ = false;
  roundMessage_ = "";
  cout << rightnow() << "Manager created for group "
    << group.url << " (\"" << group.name << "\")" << endl;  
}


Manager::~Manager(){
  uv_mutex_destroy(&classMutex_);
  uv_mutex_destroy(&propertyMutex_);
}


// TODO: should be called before broadcasting SETUP
void
Manager::fillMemberSchedules() {
  vector< vector<sched_t>* > schedules;
  // Each member has a reference to the manager
  memb_iter mitr = members.begin();
  for (; mitr != members.end(); ++mitr) {
    schedules.push_back(&mitr->second.schedule);
  }

  Crypto::fillDisjointVectors(schedules, XBSCHEDULESIZE);

#ifdef DEBUG
  cout << "after fillDisjointVectors:\n";
  unsigned i, j;
  for (i = 0; i < schedules.size(); i++) {
    cout << i << ":\n";
    for (j = 0; j < schedules.at(i)->size(); j++) {
      cout << (*schedules.at(i))[j] << ", ";
    }
    cout << endl;
  }
#endif
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
Manager::canDeliverSchedules() {
  if (schedulesDelivered_) {
    return false;
  }
  memb_iter mitr = members.begin();
  for (; mitr != members.end(); ++mitr) {
    if (!mitr->second.present || !mitr->second.ready) {
      return false;
    }
  }
  return true;
}


bool
Manager::canStartChat() { // not threadsafe
  if (chatStarted_) {
    return false;
  }
  memb_iter mitr = members.begin();
  for (; mitr != members.end(); ++mitr) {
    if (!mitr->second.present ||
      !mitr->second.ready || !mitr->second.clientHasSchedule) {
      return false;
    }
  }
  return true;
}


bool
Manager::allMessagesProcessed() { // not threadsafe
  memb_iter mitr = members.begin();
  for (; mitr != members.end(); ++mitr) {
    if (!mitr->second.messageProcessed) {
      return false;
    }
  }
  return true;
}


void
Manager::setRoundMessage(string msg) {
  uv_mutex_lock(&propertyMutex_);
  roundMessage_ = msg;
  roundIsImportant_ = true;
  uv_mutex_unlock(&propertyMutex_);
}


void
Manager::startChatIfNecessary() {
  uv_mutex_lock(&classMutex_);

  if (canStartChat()) {
    uv_work_t *req = ALLOC(uv_work_t);
    req->data = this;
    uv_queue_work(loop, req, onStartChatWork,
      (uv_after_work_cb)afterRoundWork);
  }
  uv_mutex_unlock(&classMutex_);
}


void
Manager::deliverSchedulesIfNecessary() {
  uv_mutex_lock(&classMutex_);

  if (canDeliverSchedules()) {
    uv_work_t *req = ALLOC(uv_work_t);
    req->data = this;
    uv_queue_work(loop, req, onSetupWork,
      (uv_after_work_cb)afterSetupWork);
  }

  uv_mutex_unlock(&classMutex_);
}


void
Manager::broadcastIfNecessary() {
  uv_mutex_lock(&classMutex_);

  if (allMessagesProcessed()) {
    uv_work_t *req = ALLOC(uv_work_t);
    req->data = this;
    uv_queue_work(loop, req, onBroadcastWork,
      (uv_after_work_cb)afterRoundWork);
  }

  uv_mutex_unlock(&classMutex_);
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
  cout << rightnow() << "inside getStartChatBuffers " <<
    "for " << group.name << endl;
#endif
  uv_mutex_lock(&classMutex_);

  if (!chatStarted_) {
    cout << rightnow() << 
      "getting start chat buffers for " << group.name << endl;

    memb_iter mitr = members.begin();
    for (; mitr != members.end(); ++mitr) {
      mitr->second.baton->getStartChat();            
    }
    chatStarted_ = true;
  }    

  uv_mutex_unlock(&classMutex_);
}


void
Manager::getSetupBuffers() {
#ifdef TRACE
  cout << rightnow() << "inside getSetupBuffers " <<
    "for " << group.name << endl;
#endif
  uv_mutex_lock(&classMutex_);

  if (!schedulesDelivered_) {
    cout << rightnow() << 
      "getting setup buffers for " << group.name << endl;

    fillMemberSchedules();

    memb_iter mitr = members.begin();
    for (; mitr != members.end(); ++mitr) {
      mitr->second.baton->getSetup();            
    }
    schedulesDelivered_ = true;
  }    

  uv_mutex_unlock(&classMutex_);
}


void
Manager::broadcast() {

    uv_mutex_lock(&classMutex_);

    cout << rightnow() << "broadcasting start chat to " << group.name << endl;

    memb_iter mitr = members.begin();
    for (; mitr != members.end(); ++mitr) {
      mitr->second.baton->unicast();            
    }

    uv_mutex_unlock(&classMutex_);
}


void
Manager::onStartChatWork(uv_work_t *r) {
  Manager *mgr = reinterpret_cast<Manager *>(r->data);
  mgr->getStartChatBuffers();
}


void
Manager::onSetupWork(uv_work_t *r) {
  Manager *mgr = reinterpret_cast<Manager *>(r->data);
  mgr->getSetupBuffers();
}


void
Manager::onBroadcastWork(uv_work_t *r) {
  Manager *mgr = reinterpret_cast<Manager *>(r->data);
  mgr->getStartChatBuffers();
}


void
Manager::afterRoundWork(uv_work_t *r) {
  Manager *mgr = reinterpret_cast<Manager *>(r->data);
  mgr->broadcast();
  mgr->roundIsImportant_ = false;
  r->data = NULL;
  free(r);
}


void
Manager::afterSetupWork(uv_work_t *r) {
  Manager *mgr = reinterpret_cast<Manager *>(r->data);
  mgr->broadcast();
  r->data = NULL;
  free(r);
}


void
Manager::endChat() {
  // TODO: may want to handle member.present 
  // and member.ready here instead of in baton destructor
  if (chatStarted_) {
    uv_mutex_lock(&classMutex_);
    if (chatStarted_) {
      cout << rightnow() << "ending chat for " << group.name << endl;
      chatStarted_ = false;
      schedulesDelivered_ = false;
    }
    uv_mutex_unlock(&classMutex_);
  }
}


// NOTE: mutex may not actually be necessary here
sched_t
Manager::getTargetModulo() {
  if (!moduloCalculated_) {
    targetModulo_ = (Crypto::generateRandomInt<sched_t>() % members.size());
    moduloCalculated_ = true;
  }
  return targetModulo_;
}


string
Manager::decryptSessionMessage(string& ciphertext) {
  string r = Crypto::hybridDecrypt(privateKey_, ciphertext);
  return r;
}

// static
bool
Manager::memberCanTransmit(Manager *mgr, Member *member) {
  bool retval = mgr->targetModulo_ == member->schedule[mgr->currentRound_];
  return retval;
}


} // namespace xblab
  