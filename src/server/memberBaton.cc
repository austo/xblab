#include <cstdio>
#include <cstdlib>

#include <pcrecpp.h>

#include <stdexcept>

#include "memberBaton.h"
#include "batonUtil.h"
#include "common/macros.h"
#include "common/common.h"

using namespace std;


namespace xblab {


extern uv_buf_t allocBuf(uv_handle_t *handle, size_t suggested_size);
extern uv_loop_t *loop;


MemberBaton::MemberBaton() {
  uvClient.data = this;
  member = NULL; // memberBaton does not own member
  if (uv_mutex_init(&mutex) != XBGOOD) {
    fprintf(stderr, "Error initializing MemberBaton mutex\n");
    throw runtime_error("Error initializing MemberBaton mutex\n");
  }
}


MemberBaton::~MemberBaton() {
  if (member != NULL) {
    member->present = false;
    fprintf(stdout, "%s%s left group %s\n",
      rightnow().c_str(), member->username.c_str(), url.c_str());  
  }
  else {
    pcrecpp::RE("\\.$").Replace(": ", &err);    
    fprintf(stdout, "%sMemberBaton being deleted.\n", err.c_str());
  }
  uv_mutex_destroy(&mutex);  
}


void
MemberBaton::needCredentialCb(uv_write_t *req, int status) {
  if (status == -1) {
    fprintf(stderr, "Write error %s\n",
      uv_err_name(uv_last_error(loop)));
    return;
  }
  MemberBaton *baton = reinterpret_cast<MemberBaton *>(req->data);
  uv_read_start(
    (uv_stream_t*) &baton->uvClient,
    allocBuf,
    baton->uvReadCb
  );
}


void
MemberBaton::setNeedCredentialCb() {
  this->uvWriteCb = needCredentialCb;
}


bool
MemberBaton::hasMember() {
  return this->member != NULL;
}


void
MemberBaton::processTransmission() {
  BatonUtil::processTransmission(this);
  // TODO: set uvWriteCb here if all went well
}


/* Message buffer methods */
void
MemberBaton::getNeedCredential() {
  BatonUtil::needCredBuf(this);    
}


// Thses methods may overlap if all members arrive at once
void
MemberBaton::getGroupEntry() {
  uv_mutex_lock(&mutex);
  BatonUtil::groupEntryBuf(this);
  uv_mutex_unlock(&mutex);
}


void
MemberBaton::getStartChat() {
  uv_mutex_lock(&mutex);
  BatonUtil::startChatBuf(this);
  uv_mutex_unlock(&mutex);
}

// TODO: use it or lose it...
// void
// MemberBaton::getNoOp(string what) {
//   BatonUtil::noOpBuf(this, what);
// }

} // namespace xblab