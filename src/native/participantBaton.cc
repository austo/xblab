#include <cstdio>
#include <cstdlib>

#include <string>

#include "native/participantBaton.h"
#include "native/participantUtil.h"
#include "crypto.h"
#include "macros.h"

namespace xblab {

ParticipantBaton::ParticipantBaton(uv_connect_t *req){
  uvServer = req->handle;
  needsJsCallback = false;  
}


ParticipantBaton::ParticipantBaton(){
  uvConnect.data = this;
  needsJsCallback = false;    
}


ParticipantBaton::ParticipantBaton(uv_connect_cb cb){
  uvConnect.data = this;
  uvConnectCb = cb;
  needsJsCallback = false;  
}


ParticipantBaton::~ParticipantBaton() {
  fprintf(stdout, "%s%s left group %s\n",
    rightnow().c_str(), participant.username.c_str(), url.c_str());  
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
  ParticipantUtil::packageCredential(this);
}

} // namespace xblab