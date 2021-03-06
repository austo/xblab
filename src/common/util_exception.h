#ifndef UTIL_EXCEPTION_H
#define UTIL_EXCEPTION_H

#include <string>
#include <exception>

namespace xblab {

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


} //namespace xblab

#endif