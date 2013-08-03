#ifndef BATON_UTIL_H
#define BATON_UTIL_H

#include <string>
#include <vector>

#include "common/util_exception.h"
#include "protobuf/xblab.pb.h"
#include "client/memberBaton.h"


namespace xblab {

// Private class - only accessible from ParticipantBaton
class BatonUtil {
  friend class MemberBaton;

  static void
  packageCredential(MemberBaton *baton);

  static void
  packageTransmission(MemberBaton *baton);

  static void
  digestBroadcast(MemberBaton *baton);

  static void
  enterGroup(MemberBaton *baton, const Broadcast::Data& data);

  static void
  startChat(MemberBaton *baton, const Broadcast::Data& data);

  static void
  chatReady(MemberBaton *baton);

  static void
  signData(Transmission&, Transmission::Data*, std::string&);

  static void
  serializeToBuffer(
    MemberBaton *baton, Transmission& trans, bool useSessionKey = false);

  BatonUtil(){};
  ~BatonUtil(){};
};

} //namespace xblab


#endif