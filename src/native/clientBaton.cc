#include <cstdio>
#include <cstdlib>

#include "clientBaton.h"
#include "util.h"

using namespace std;

namespace xblab {

extern uv_buf_t allocBuf(uv_handle_t *handle, size_t suggested_size);
extern uv_loop_t *loop;

void
ClientBaton::needCredentialCb(uv_write_t *req, int status){
  if (status == -1) {
    fprintf(stderr, "Write error %s\n",
      uv_err_name(uv_last_error(loop)));
    return;
  }
  ClientBaton *baton = reinterpret_cast<ClientBaton *>(req->data);
  uv_read_start(
    (uv_stream_t*) &baton->uvClient,
    allocBuf,
    baton->uvReadCb
  );
}

void
ClientBaton::setNeedCredentialCb(){
  this->uvWriteCb = needCredentialCb;
}


bool
ClientBaton::hasMember(){
  return this->member != NULL;
}


void
ClientBaton::initializeMember(){
  Util::initializeMember(this);
  // TODO: set uvWriteCb here if all went well
  // Welcome to the group
}


/* Message buffer methods */
void
ClientBaton::getNeedCredential(){
  Util::needCredBuf(this);    
}


void
ClientBaton::getGroupEntry(){
  Util::groupEntryBuf(this);
}


// void
// ClientBaton::getNoOp(string what) {
//   Util::noOpBuf(this, what);
// }

} // namespace xblab