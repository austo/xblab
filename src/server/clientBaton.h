#ifndef CLIENT_BATON_H
#define CLIENT_BATON_H

#include <uv.h>
#include "common/baton.h"
#include "member.h"

namespace xblab {

class ClientBaton : public DataBaton {
public:
  ClientBaton(){
    uvClient.data = this;
    member = NULL; // clientBaton does not own member
  }
  ~ClientBaton(); 

  // TODO: add uv_work_cb and uv_after_work_cb?
   
  Member *member;

  bool hasMember();
  void initializeMember();

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