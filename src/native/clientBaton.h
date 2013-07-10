#ifndef CLIENT_BATON_H
#define CLIENT_BATON_H

#include <uv.h>
#include "baton.h"
#include "member.h"

namespace xblab {

class ClientBaton : public DataBaton {
public:
    ClientBaton(){
        uvClient.data = this;
        uvWrite.data = this;
        member = NULL;
    }
    ~ClientBaton(){ }

    uv_tcp_t uvClient;
    uv_stream_t *uvServer;
    uv_buf_t uvBuf;
    uv_write_t uvWrite;
    uv_write_cb uvWriteCb; // where we go from here
    uv_read_cb uvReadCb;

    // TODO: add uv_work_cb and uv_after_work_cb?
   
    Member *member;

    void stringifyBuffer();
    bool hasMember();
    void initializeMember();

    // message buffer methods
    void getNeedCredential();
    void getGroupEntry();

    // uv_write_cb's:
    static void needCredentialCb(uv_write_t *req, int status);

};

} // namespace xblab

#endif