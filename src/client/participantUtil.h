#ifndef PARTICIPANT_UTIL_H
#define PARTICIPANT_UTIL_H

#include <string>

#include "common/util_exception.h"
#include "protobuf/xblab.pb.h"
#include "client/participantBaton.h"


namespace xblab {

// Private class - only accessible from ParticipantBaton
class ParticipantUtil {
  friend class ParticipantBaton;

  static void
  packageCredential(ParticipantBaton *baton);

  static void
  digestBroadcast(ParticipantBaton *baton);
  
  ParticipantUtil(){};
  ~ParticipantUtil(){};

  static void
  enterGroup(ParticipantBaton *baton, const Broadcast::Data& data);  

};

} //namespace xblab


#endif