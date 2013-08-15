#include <cstdio>
#include <cstdlib>

#include <pcrecpp.h>

#include <stdexcept>

#include "memberBaton.h"
#include "server.h"
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
    member->ready = false;
    member->clientHasSchedule = false;
    fprintf(stdout, "%s%s left group %s\n",
      rightnow().c_str(), member->username.c_str(), url.c_str());  
  }
  else {
    pcrecpp::RE("\\.$").Replace(": ", &err);
    fprintf(stdout, "%sMemberBaton being deleted.\n", err.c_str());
  }
  uv_mutex_destroy(&mutex);  
}


bool
MemberBaton::hasMember() {
  return this->member != NULL;
}


void
MemberBaton::processTransmission() {
  BatonUtil::processTransmission(this);
  // TODO: set uvWriteCb here if necessary
}


/* Message buffer methods */

void
MemberBaton::getNeedCredential() {
  BatonUtil::needCredBuf(this);    
}


// TODO: mutex may be unnecessary
// May overlap if all members arrive at once
void
MemberBaton::getGroupEntry() {
  uv_mutex_lock(&mutex);
  BatonUtil::groupEntryBuf(this);
  uv_mutex_unlock(&mutex);
}


void
MemberBaton::getSetup() {
  BatonUtil::setupBuf(this);
}

// Should be called from uv_work_cb if used with uv_queue_work
void
MemberBaton::getStartChat() {
  BatonUtil::startChatBuf(this);
}


void
MemberBaton::getMessage() {
  BatonUtil::messageBuf(this);
}


// Should be called from uv_after_work_cb if used with uv_queue_work
// TODO: move to BatonUtil?
void
MemberBaton::unicast() {
  size_t len = xBuffer.size();
  // allocate contiguous block to enable single free call in on_write
  uv_write_t *req = (uv_write_t*)malloc(sizeof(uv_write_t) + len);
  void *bufStart = req + 1;
  memcpy(bufStart, &xBuffer[0], len);
  uv_buf_t buf = uv_buf_init((char*)bufStart, len);
#ifdef DEBUG
  fprintf(stdout, "%sunicasting to %s...\n",
      rightnow().c_str(), member->username.c_str());  
#endif
  uv_write(req, (uv_stream_t*)&uvClient, &buf, 1, on_write);
}

} // namespace xblab