
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
#include "protobuf/xblab.pb.h"


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

  void
  fillMemberSchedules();

  bool
  allMembersPresent() const;

  bool
  canStartChat() const;

  bool
  allMessagesProcessed() const;

  bool
  canDeliverSchedules() const;

  bool
  getRoundIsImportant() {
    return flags.roundIsImportant;
  }

  void
  respondAfterRead();

  void
  startChatIfNecessary(bool lock = false);

  void
  deliverSchedulesIfNecessary(bool lock = false);

  void
  broadcastIfNecessary(bool lock = false);

  void
  broadcastRoundIfNecessary();

  void
  getStartChatBuffers();

  void
  getSetupBuffers();

  void
  getMessageBuffers(bool lock = false);

  void
  processMemberMessage(int memberId, std::string& datastr,
    std::string signature, const Transmission::Payload& payload);

  void
  broadcast(bool lock);

  void
  endChat();

  std::string
  getPrivateKey() {
    return privateKey_;
  }

  std::string
  getRoundMessage() {
    return roundMessage_;
  }

  void
  setRoundMessage(std::string msg);

  std::string
  decryptSessionMessage(std::string& ciphertext);

  static bool
  memberCanTransmit(Manager *mgr, Member *member);

  // static uv_work_cb
  static void
  onStartChatWork(uv_work_t *r);

  static void
  onBroadcastWork(uv_work_t *r);

  static void
  onSetupWork(uv_work_t *r);


  // static uv_after_work_cb
  static void
  afterRoundWork(uv_work_t *r);

  static void
  afterSetupWork(uv_work_t *r);
  
private:
  int currentRound_;
  sched_t targetModulo_;

  std::string privateKey_;
  std::string roundMessage_;

  uv_mutex_t classMutex_;
  uv_mutex_t propertyMutex_;

  struct stateFlags {
    bool moduloCalculated;
    bool chatStarted;
    bool roundIsImportant;
    bool schedulesDelivered;
    bool messagesDelivered;

    stateFlags() :
      moduloCalculated(false),
      chatStarted(false),
      roundIsImportant(false),
      schedulesDelivered(false),
      messagesDelivered(false) { }

    void
    resetRound() {
      roundIsImportant = false;
      messagesDelivered = false;
      moduloCalculated = false;
    }

    void
    reset() {
      chatStarted = false;
      schedulesDelivered = false;
      roundIsImportant = false;
      messagesDelivered = false;
      moduloCalculated = false;
    }

  };

  stateFlags flags;

  template <class T>
  void
  cleanMemberSchedules(
    std::vector< std::vector<T>* >& schedules, size_t n);
};

} //namespace xblab

#endif