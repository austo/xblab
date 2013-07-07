#ifndef CLIENT_BATON_H
#define CLIENT_BATON_H

#include <uv.h>
#include "baton.h"
#include "member.h"
#include "manager.h"


namespace xblab {

class ClientBaton : public DataBaton {
public:
    ClientBaton(){
        uvClient.data = this;
        uvWrite.data = this;
    }
    ~ClientBaton(){ }

    uv_tcp_t uvClient;
    uv_stream_t *uvServer;
    uv_buf_t uvBuf;
    uv_write_t uvWrite;
   
    Member *member;
};

} // namespace xblab

#endif