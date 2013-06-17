#ifndef __DB_H
#define __DB_H

#include <string>
#include <map>
#include <node.h>
#include <exception>
#include "group.h"
#include "member.h"

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

class Db {
    public:
        Db();
        ~Db();
        static Group get_group(std::string url);
        static std::map<int, Member> get_members(int group_id);
        static std::string conn_str();        

        //static int authenticate_member(std::string, std::string, int);

    private:
};
}//namespace xblab

#endif