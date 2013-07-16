#ifndef BATON_UTIL_H
#define BATON_UTIL_H

#include <string>

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
  digestBroadcast(MemberBaton *baton);
  
  BatonUtil(){};
  ~BatonUtil(){};

  static void
  enterGroup(MemberBaton *baton, const Broadcast::Data& data);  

};

} //namespace xblab


#endif