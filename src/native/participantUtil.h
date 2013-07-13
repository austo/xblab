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

class ParticipantUtil {

public:   
  friend class participantBaton;

  static void
  packageCredential(ParticipantBaton *baton);

  static void
  digestBroadcast(ParticipantBaton *baton);

  
private:
  ParticipantUtil(){};
  ~ParticipantUtil(){};

  static void
  enterGroup(ParticipantBaton *baton, const Broadcast::Data& data);  
  
};

} //namespace xblab


#endif