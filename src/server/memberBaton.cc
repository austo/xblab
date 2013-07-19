#include <cstdio>
#include <cstdlib>

#include <regex.h>

#include "memberBaton.h"
#include "batonUtil.h"
#include "common/macros.h"

using namespace std;

namespace xblab {

extern uv_buf_t allocBuf(uv_handle_t *handle, size_t suggested_size);
extern uv_loop_t *loop;


MemberBaton::~MemberBaton() {
  if (member != NULL) {
    member->present = false;
    fprintf(stdout, "%s%s left group %s\n",
      rightnow().c_str(), member->username.c_str(), url.c_str());  
  }
  else {
    // TODO: inline
    regex_t regex;
    int res;
    res = regcomp(&regex, ".$", 0);
    res = regexec(&regex, err.c_str(), 0, NULL, 0);
    if (res == XBGOOD) {
      err.erase(err.size() - 1, 1);
    }
    fprintf(stdout, "%s: MemberBaton being deleted.\n", err.c_str());
    regfree(&regex);
  }  
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
MemberBaton::initializeMember() {
  BatonUtil::initializeMember(this);
  // TODO: set uvWriteCb here if all went well
  // Welcome to the group
}


/* Message buffer methods */
void
MemberBaton::getNeedCredential() {
  BatonUtil::needCredBuf(this);    
}


void
MemberBaton::getGroupEntry() {
  BatonUtil::groupEntryBuf(this);
}


// void
// MemberBaton::getNoOp(string what) {
//   BatonUtil::noOpBuf(this, what);
// }

} // namespace xblab