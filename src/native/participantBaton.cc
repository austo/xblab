#include <cstdio>
#include <cstdlib>

#include <string>

#include "native/participantBaton.h"
#include "native/participantUtil.h"
#include "crypto.h"

namespace xblab {

ParticipantBaton::ParticipantBaton(uv_connect_t *req){
  uvServer = req->handle;
  uvWrite.data = this;
  needsJsCallback = false;  
}


ParticipantBaton::ParticipantBaton(){
  uvWrite.data = this;
  uvConnect.data = this;
  needsJsCallback = false;    
}


ParticipantBaton::ParticipantBaton(uv_connect_cb cb){
  uvWrite.data = this;
  uvConnect.data = this;
  uvConnectCb = cb;
  needsJsCallback = false;  
}


// Read current contents of uvBuf into xBuffer
void
ParticipantBaton::stringifyBuffer(){
  this->xBuffer = std::string(this->uvBuf.base, this->uvBuf.len);
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


void 
ParticipantBaton::packageCredential(){
  // stringifyBuffer();
  ParticipantUtil::packageCredential(this);
}

} // namespace xblab