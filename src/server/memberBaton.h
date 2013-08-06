#ifndef MEMBER_BATON_H
#define MEMBER_BATON_H

#include <uv.h>
#include "common/baton.h"
#include "member.h"

namespace xblab {

class MemberBaton : public DataBaton {
public:
  MemberBaton();
  ~MemberBaton(); 

  
  // Note to self: make sure to check this on error
  Member *member;
  uv_mutex_t mutex; // thread-safe access to shared data
  
  bool hasMember();
  void processTransmission();

  // message buffer methods
  void getNeedCredential();
  void getGroupEntry();
  void getStartChat(); 

};

} // namespace xblab

#endif