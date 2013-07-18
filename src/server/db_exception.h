#ifndef DB_EXCEPTION_H
#define DB_EXCEPTION_H

#include <string>
#include <exception>

namespace xblab {

class db_exception : public std::exception {
public:
  db_exception(){
    message_ = std::string("Database layer exception.");
  }
  db_exception(std::string err_msg){
    message_ = err_msg;
  }
  ~db_exception() throw(){};
  virtual const char* what() const throw(){
    return this->message_.c_str();
  }
private:
  std::string message_;        
};

} // namespace xblab

#endif