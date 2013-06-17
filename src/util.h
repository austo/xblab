#ifndef __UTIL_H
#define __UTIL_H

#include <string>
#include <map>
#include <node.h>
#include <exception>
#include "group.h"
#include "member.h"

namespace xblab {

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
        Util();
        ~Util();
        
        #ifndef XBLAB_CLIENT
 
        static std::string get_need_cred_buf();

        #endif
        
        static std::string parse_buf(std::string in);

        static std::string v8_to_string(v8::Local<v8::Value> value) {
            v8::String::Utf8Value utf8_value(value);
            return std::string(*utf8_value);
        }


        //static int authenticate_member(std::string, std::string, int);

    private:
        
};

} //namespace xblab


#endif