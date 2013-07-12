#ifndef UTIL_H
#define UTIL_H

#include <string>
#include <map>
#include <exception>
#include "protobuf/xblab.pb.h"
#include "participantBaton.h"


namespace xblab {

// Message types enum - global to xblab namespace

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

  static MessageType parseBroadcast(participantBaton *baton);
  static std::string packageParticipantCredentials(participantBaton *baton);

private:
  Util(){};
  ~Util(){};  
};

} //namespace xblab


#endif