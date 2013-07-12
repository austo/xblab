#ifndef BATON_H
#define BATON_H

#include <string>
#include <uv.h>

namespace xblab {

class DataBaton {
public:
  DataBaton(){
    uvWork.data = this;
    uvClient.data = this;
    err = "";
  }
  virtual ~DataBaton(){ }
  uv_tcp_t uvClient;
  uv_work_t uvWork;
  std::string xBuffer;
  std::string nonce;
  std::string returnNonce;
  std::string url;
  std::string err;
  void *auxData;
};

} // namespace xblab

#endif
