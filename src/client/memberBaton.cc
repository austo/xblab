#include <cstdio>
#include <cstdlib>

#include <string>

#include "client/memberBaton.h"
#include "client/batonUtil.h"
#include "common/crypto.h"
#include "common/common.h"
#include "common/macros.h"

namespace xblab {

MemberBaton::MemberBaton(uv_connect_t *req) {
  uvServer = req->handle;
  needsJsCallback = false;  
}


MemberBaton::MemberBaton() {
  uvConnect.data = this;
  needsJsCallback = false;    
}


MemberBaton::MemberBaton(uv_connect_cb cb) {
  uvConnect.data = this;
  uvConnectCb = cb;
  needsJsCallback = false;  
}


MemberBaton::~MemberBaton() {
  fprintf(stdout, "%s%s left group %s\n",
    rightnow().c_str(), member.username.c_str(), url.c_str());  
}


bool
MemberBaton::hasKeys() {
  return this->member.hasKeys;
}


void
MemberBaton::getKeys() {
  Crypto::generateKey(
    this->member.privateKey,
    this->member.publicKey
  );
}


void
MemberBaton::digestBroadcast() {
  this->stringifyBuffer();
  BatonUtil::digestBroadcast(this);
}


void 
MemberBaton::packageCredential() {
  BatonUtil::packageCredential(this);
}

} // namespace xblab