#ifndef BATON_H
#define BATON_H

#include <cstdlib>
#include <string>
#include <uv.h>

namespace xblab {

class DataBaton {
public:
  DataBaton(){
    uvWork.data = this;
    uvClient.data = this;
    uvWrite.data = this;
    err = "";
    uvBuf.base = NULL;
    uvBuf.len = 0;
    needsUvWrite = false;
  }

  virtual ~DataBaton(){ }

  uv_tcp_t uvClient;
  uv_stream_t *uvServer;
  uv_work_t uvWork;
  uv_buf_t uvBuf;
  uv_write_t uvWrite;
  uv_write_cb uvWriteCb; // where we go from here
  uv_read_cb uvReadCb;

  std::string xBuffer;
  std::string nonce;
  std::string returnNonce;
  std::string url;
  std::string err;
  bool needsUvWrite;

  void *auxData;


  // Read current contents of uvBuf into xBuffer and free uvBuf.base
  void stringifyBuffer(){
    this->xBuffer = std::string(this->uvBuf.base, this->uvBuf.len);
    free(this->uvBuf.base);
    this->uvBuf.base = NULL;
    this->uvBuf.len = 0;
  }

};

} // namespace xblab

#endif
