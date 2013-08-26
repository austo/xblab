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
#include "common/logger.h"


using namespace std;

namespace xblab {

extern uv_loop_t *loop;

typedef map<int, Member>::iterator memb_iter;
typedef map<int, Member>::const_iterator const_memb_iter;

template <class T>
void
Manager::cleanMemberSchedules(
  std::vector< std::vector<T>* >& schedules, size_t n) {
  if (schedules.size() < 2) { return; }

  T i = 0, s = 0, curr;  
  std::vector<T> *firstSched = schedules.at(s);

  for (; i < n; i++) {
    curr = (*firstSched)[i];

  #ifdef TRACE
    std::cout << "\n(0) " << curr << ", ";
  #endif

    for (s = 1; s < schedules.size(); s++) {

    #ifdef TRACE
      std::cout << "(" << s << ") " << (*schedules.at(s))[i] << ", ";
    #endif

      if ((*schedules.at(s))[i] == curr) {
        (*schedules.at(s))[i] += 1;
        ++curr;

      #ifdef TRACE
        std::cout << "\nincremented curr: " << curr << std::endl;
      #endif

      }
    }
  }
}


Manager::Manager(string url) {

  INIT_MUTEX(classMutex_, "Manager: class");
  INIT_MUTEX(propertyMutex_, "Manager: property");

  Crypto::generateKey(this->privateKey_, this->publicKey);
  group = Db::getGroup(url);
  
  // We've got the room ID, now get our members
  members = Db::getMembers(group.id);  

  memb_iter mitr = members.begin();
  for (; mitr != members.end(); ++mitr) {
    mitr->second.manager = this;
  }

  currentRound_ = 0;
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

#ifdef TRACE
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
Manager::allMembersPresent() const {
  const_memb_iter mitr = members.begin();
  for (; mitr != members.end(); ++mitr) {
    if (!mitr->second.present) {
      return false;
    }
  }
  return true;
}


bool
Manager::canDeliverSchedules() const {
  if (flags.schedulesDelivered) {
    return false;
  }
  const_memb_iter mitr = members.begin();
  for (; mitr != members.end(); ++mitr) {
    if (!mitr->second.present || !mitr->second.ready) {
      return false;
    }
  }
  return true;
}


bool
Manager::canStartChat() const {
  if (flags.chatStarted) {
    return false;
  }
  const_memb_iter mitr = members.begin();
  for (; mitr != members.end(); ++mitr) {
    if (!mitr->second.present ||
      !mitr->second.ready || !mitr->second.clientHasSchedule) {
      return false;
    }
  }
  return true;
}


bool
Manager::allMessagesProcessed() const {
  if (flags.messagesDelivered) {
    return false;
  }
  const_memb_iter mitr = members.begin();
  for (; mitr != members.end(); ++mitr) {
    if (!mitr->second.messageProcessed) {
      cout << mitr->second.handle << " message not processed\n";
      return false;
    }
    if (!mitr->second.present) {
      cout << mitr->second.handle << " not present\n";
      return false;
    }
    if (!mitr->second.ready) {
      cout << mitr->second.handle << " not ready\n";
      return false;
    }
    if (!mitr->second.clientHasSchedule) {
      cout << mitr->second.handle << " does not have schedule\n";
      return false;
    }
  }
  return true;
}


void
Manager::setRoundMessage(string msg) { // not threadsafe
  roundMessage_ = msg;
  flags.roundIsImportant = true;
}


void
Manager::startChatIfNecessary(bool lock) {
  if (lock) {
    uv_mutex_lock(&classMutex_);
  }

  if (canStartChat()) {
    uv_work_t *req = ALLOC(uv_work_t);
    req->data = this;
    uv_queue_work(loop, req, onStartChatWork,
      (uv_after_work_cb)afterRoundWork);
  }
  if (lock) {
    uv_mutex_unlock(&classMutex_);
  }
}


void
Manager::deliverSchedulesIfNecessary(bool lock) {
  if (lock) {
    uv_mutex_lock(&classMutex_);
  }

  if (canDeliverSchedules()) {
    uv_work_t *req = ALLOC(uv_work_t);
    req->data = this;
    uv_queue_work(loop, req, onSetupWork,
      (uv_after_work_cb)afterSetupWork);
  }
  if (lock) {
    uv_mutex_unlock(&classMutex_);
  }
}


// NOTE: this should probably be called from a uv_async send
void
Manager::broadcastIfNecessary(bool lock) {
  if (lock) {
    uv_mutex_lock(&classMutex_);
  }

  if (allMessagesProcessed()) {
    uv_work_t *req = ALLOC(uv_work_t);
    req->data = this;
    uv_queue_work(loop, req, onBroadcastWork,
      (uv_after_work_cb)afterRoundWork);
  }  
  if (lock) {
    uv_mutex_unlock(&classMutex_);
  }
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

  if (!flags.chatStarted) {
    cout << rightnow() << 
      "getting start chat buffers for " << group.name << endl;

    memb_iter mitr = members.begin();
    for (; mitr != members.end(); ++mitr) {
      mitr->second.baton->getStartChat();            
    }
    flags.chatStarted = true;
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

  if (!flags.schedulesDelivered) {
    cout << rightnow() << 
      "getting setup buffers for " << group.name << endl;

    fillMemberSchedules();

    memb_iter mitr = members.begin();
    for (; mitr != members.end(); ++mitr) {
      mitr->second.baton->getSetup();            
    }
    flags.schedulesDelivered = true;
  }    

  uv_mutex_unlock(&classMutex_);
}


void
Manager::getMessageBuffers(bool lock) {
#ifdef TRACE
  // logging
#endif
  if (lock) {
    uv_mutex_lock(&classMutex_);
  }

  if (!flags.messagesDelivered) {
    cout << rightnow() << 
      "getting message buffers for " << group.name << endl;    

    memb_iter mitr = members.begin();
    for (; mitr != members.end(); ++mitr) {
      mitr->second.baton->getMessage();            
    }
    flags.messagesDelivered = true;
  }

  if (lock) {
    uv_mutex_unlock(&classMutex_);
  }
}


void
Manager::processMemberMessage(int memberId, string& datastr,
  string signature, const Transmission::Payload& payload) {
  
  /* Can user broadcast?
   * If so, verify message.
   * If message is okay, is the message important?
   * If so, set manager round message and broadcast with next round modulo.
   * If not, broadcast no message with next round modulo. 
   */

  uv_mutex_lock(&classMutex_);

  Member& member = members[memberId];

#ifdef TRACE
  cout << "processMemberMessage for " << member.handle << endl;
#endif

  if (memberCanTransmit(this, &member)) {
    FileLogger logger;
    logger.setFile(logname());
    
    if (!Crypto::verify(member.publicKey, datastr, signature)) {       
      F_LOG(logger, ERROR) << "offending public key for " <<
        member.handle << ":\n" <<
        member.publicKey << endl <<
        "signature:\n" << signature << endl << 
        "msg:\n" << datastr;      
      // throw util_exception("User key not verified.");
    }    
    if (payload.is_important()) {
      
      setRoundMessage(payload.content());
      F_LOG(logger, DEBUG) << "round " << currentRound_ << ": " <<
        member.handle << " says " << payload.content();   
    }
    else {      
      F_LOG(logger, DEBUG) << "round " << currentRound_ << ": " <<
        member.handle << " has nothing to say.";
    }
    flags.messagesDelivered = false;
  }

#ifdef TRACE
  cout << "setting messageProcessed for " << member.handle << " to true.\n";
#endif

  member.messageProcessed = true;

  if (payload.need_schedule()) {
    member.clientHasSchedule = false;
    flags.schedulesDelivered = false;
    // TODO: wait for all schedule reqs then send setup
  } 

  // TODO: seems to break when refactored...
  // broadcastRoundIfNecessary();
  if (allMessagesProcessed()) {
    getMessageBuffers();

    memb_iter mitr = members.begin();
    for (; mitr != members.end(); ++mitr) {
      mitr->second.baton->unicast();            
    }

    ++currentRound_;
    flags.resetRound();

  }
  else {
    cout << "all messages not processed\n";
  }

  uv_mutex_unlock(&classMutex_);

}


void
Manager::broadcastRoundIfNecessary() {
  if (allMessagesProcessed()) {
    getMessageBuffers();

    broadcast(false);

    ++currentRound_;
    flags.resetRound();

  }
  else {
    cout << "all messages not processed\n";
  }
}


void
Manager::respondAfterRead() {
  
  uv_mutex_lock(&classMutex_);

  if (allMessagesProcessed()) {
    broadcastIfNecessary();
  }  
  else if (canStartChat()) {
    startChatIfNecessary();
  }
  else if (canDeliverSchedules()) {
    deliverSchedulesIfNecessary();
  }

  uv_mutex_unlock(&classMutex_);
}


void
Manager::broadcast(bool lock) {

  if (lock) {
    uv_mutex_lock(&classMutex_);
  }

  memb_iter mitr = members.begin();
  for (; mitr != members.end(); ++mitr) {
    mitr->second.baton->unicast();            
  }

  if (lock) {
    uv_mutex_unlock(&classMutex_);
  }
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
  mgr->getMessageBuffers();
}


void
Manager::afterRoundWork(uv_work_t *r) {
#ifdef TRACE
  FileLogger logger;
  logger.setFile(logname());  
#endif
  Manager *mgr = reinterpret_cast<Manager *>(r->data);
  mgr->broadcast(true);
  ++mgr->currentRound_;
#ifdef TRACE
  F_LOG(logger, DEBUG) << "round incremented to " <<
    mgr->currentRound_ << endl;
#endif
  mgr->flags.resetRound();
  r->data = NULL;
  free(r);
}


void
Manager::afterSetupWork(uv_work_t *r) {
  Manager *mgr = reinterpret_cast<Manager *>(r->data);
  mgr->broadcast(true);
  r->data = NULL;
  free(r);
}


void
Manager::endChat() {
  // TODO: may want to handle member.present 
  // and member.ready here instead of in baton destructor
  if (flags.chatStarted) {
    uv_mutex_lock(&classMutex_);
    if (flags.chatStarted) {
      cout << rightnow() << "ending chat for " << group.name << endl;
      flags.reset();
    }
    uv_mutex_unlock(&classMutex_);
  }
}


// TODO: should be const, threadsafe, with modulo calculated elsewhere
sched_t
Manager::getTargetModulo() {
  if (!flags.moduloCalculated) {
    uv_mutex_lock(&propertyMutex_);
    if (!flags.moduloCalculated) {
      targetModulo_ = (Crypto::generateRandomInt<sched_t>() % members.size());
      flags.moduloCalculated = true;
    }
    uv_mutex_unlock(&propertyMutex_);
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
  