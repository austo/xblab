#ifndef DB_H
#define DB_H

#include <string>
#include <map>

#include "group.h"
#include "member.h"

namespace xblab {

class Db {
  public:       
    static Group getGroup(std::string url);
    static Group getGroup(std::string conn, std::string url);

    static std::map<int, Member> getMembers(int group_id);
    static std::map<int, Member> getMembers(std::string conn, int group_id);

  private:
    Db(){};
    ~Db(){};
};
}//namespace xblab

#endif