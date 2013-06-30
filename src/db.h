#ifndef DB_H
#define DB_H

#include <string>
#include <map>
#include <exception>

#include <node.h>

#include "group.h"
#include "member.h"
// #include "user.h"

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
        static Group getGroup(std::string url);
        static Group getGroup(std::string conn, std::string url);

        static std::map<int, Member> getMembers(int group_id);
        static std::map<int, Member> getMembers(std::string conn, int group_id);
        static std::string connectionString();

    private:
};
}//namespace xblab

#endif