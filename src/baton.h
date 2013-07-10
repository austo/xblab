#ifndef BATON_H
#define BATON_H

#include <string>

namespace xblab {

class DataBaton {
public:
  DataBaton(){
    uvWork.data = this;
    err = "";
  }
  virtual ~DataBaton(){ }
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
