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

  // TODO: add uv_work_cb and uv_after_work_cb?
  
  // Note to self: make sure to check this on error
  Member *member;
  uv_mutex_t mutex; // enable thread-safe access to shared data
  
  bool hasMember();
  void processTransmission();

  // message buffer methods
  void getNeedCredential();
  void setNeedCredentialCb();
  void getGroupEntry();
  // void getNoOp(std::string what);

  // uv_write_cb's:
  static void needCredentialCb(uv_write_t *req, int status);


};

} // namespace xblab

#endif