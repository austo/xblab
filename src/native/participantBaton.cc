#include <cstdio>
#include <cstdlib>

#include "participantBaton.h"
#include "util.h"


namespace xblab {

ParticipantBaton(uv_connect_t *req){
  uvServer = req->handle;
  uvWrite.data = this;    
}


ParticipantBaton(){
  uvWrite.data = this;    
}


ParticipantBaton(uv_connect_cb cb){
  uvWrite.data = this;
  uvConnect.data = this;
  uvConnectCb = cb;
}

ParticipantBaton::~ParticipantBaton(){
  this->jsCallback.Dispose(); // TODO: make sure this is safe
}

// Read current contents of uvBuf into xBuffer
void
ParticipantBaton::stringifyBuffer(){
  this->xBuffer = string(this->uvBuf.base, this->uvBuf.len);
}

} // namespace xblab