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
  digestBroadcast(MemberBaton *baton);
  
  BatonUtil(){};
  ~BatonUtil(){};

  static void
  enterGroup(MemberBaton *baton, const Broadcast::Data& data);


  /* Templates */

  // template <class T>
  // static std::vector<T>
  // processSchedule(std::string& s) {
  //   return std::vector<T>((T*)&s[0], ((T*)&s[0] + s.size()));
  // }

};

} //namespace xblab


#endif