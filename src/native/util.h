#ifndef NATIVE_UTIL_H
#define NATIVE_UTIL_H

#include <string>
#include <map>
#include <exception>
#include "protobuf/xblab.pb.h"
#include "clientBaton.h"


namespace xblab {

// Message types - global to xblab namespace

enum MessageType {
  NEEDCRED,
  GROUPLIST,
  GROUPENTRY,
  BEGIN,
  BROADCAST,
  GROUPEXIT,
  QUIT,
  INVALID
};


class util_exception : public std::exception {
public:
  util_exception(){
    message_ = std::string("Data integrity exception.");
  }
  util_exception(std::string err_msg){
    message_ = err_msg;
  }
  ~util_exception() throw(){};
  virtual const char* what() const throw(){
    return this->message_.c_str();
  }
private:
  std::string message_;
};


class Util {
public:        
  friend class ClientBaton;   

private:
  Util(){ /* worker for baton */ };
  ~Util(){};

  static void
  needCredBuf(ClientBaton* baton);

  static void
  groupEntryBuf(ClientBaton* baton);

  static void
  initializeMember(ClientBaton* baton); 
     
  static void
  processCredential(ClientBaton*, std::string&,
    std::string, const Transmission::Credential&);
};

} // namespace xblab


#endif