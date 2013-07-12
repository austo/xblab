#ifndef PARTICIPANT_UTIL_H
#define PARTICIPANT_UTIL_H

#include <string>
#include <exception>
#include "protobuf/xblab.pb.h"
#include "native/participantBaton.h"


namespace xblab {

// Message types enum - global to xblab namespace

class util_exception : public std::exception {
public:
  util_exception(){
    message_ = std::string("ParticipantUtil exception");
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
  friend class participantBaton;

  static std::string packageCredential(ParticipantBaton *baton);
  static void digestBroadcast(ParticipantBaton *baton)

private:
  Util(){};
  ~Util(){};  
};

} //namespace xblab


#endif