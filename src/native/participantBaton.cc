#include <cstdio>
#include <cstdlib>

#include "native/participantBaton.h"
#include "native/participantUtil.h"
#include "crypto.h"

namespace xblab {

ParticipantBaton::ParticipantBaton(uv_connect_t *req){
  uvServer = req->handle;
  uvWrite.data = this;
  wrapperHasCallback = false;  
}


ParticipantBaton::ParticipantBaton(){
  uvWrite.data = this;  
  wrapperHasCallback = false;    
}


ParticipantBaton::ParticipantBaton(uv_connect_cb cb){
  uvWrite.data = this;
  uvConnect.data = this;
  uvConnectCb = cb;
  wrapperHasCallback = false;  
}


// Read current contents of uvBuf into xBuffer
void
ParticipantBaton::stringifyBuffer(){
  this->xBuffer = string(this->uvBuf.base, this->uvBuf.len);
}


bool
ParticipantBaton::hasKeys() {
  return this->participant.hasKeys;
}


void
ParticipantBaton::getKeys() {
  Crypto::generateKey(
    this->participant.privateKey,
    this->participant.publicKey
  );
}

void
ParticipantBaton::digestBroadcast(){
  this->stringifyBuffer();
  ParticipantUtil::digestBroadcast(this);
}

} // namespace xblab