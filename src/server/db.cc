#include <iostream>
#include <sstream>
#include <pqxx/pqxx>

#include "db.h"
#include "common/macros.h"

using namespace std;
using namespace pqxx;


namespace xblab {

extern string xbConnectionString;

// Calling code responsible for string trimming 
Group Db::getGroup(string url){
  return getGroup(xbConnectionString, url);
}

Group Db::getGroup(string conn, string url){
  connection c(conn);
  Group group;
  work txn(c);
  stringstream ss;
  ss << "select id, name from groups where url = " << txn.quote(url) << ";";

  result groups = txn.exec(ss.str());

  if (groups.size() > 1){
    throw db_exception("Multiple groups found.");
  }
  else if (groups.size() == 0){
    throw db_exception("Group not found.");
  }
  else{
    group.id = groups[0][0].as<int>();
    group.name = groups[0][1].as<string>();
    group.url = url;
  }
  return group;
}


map<int, Member> Db::getMembers(int group_id){
  return getMembers(xbConnectionString, group_id);
}

map<int, Member> Db::getMembers(string conn, int group_id){

  assert(group_id > 0);

  connection c(conn);
  map<int, Member> retval = map<int, Member>();
  work txn(c);
  stringstream ss;
  // No need to quote the string here as it's coming from previous call;
  ss << "select * from get_group_users(" << group_id << ");";
  // cout << ss.str() << endl;
  result members = txn.exec(ss.str());

  result::const_iterator row = members.begin();
  for (; row != members.end(); ++row){
    retval[row["id"].as<int>()] = 
      Member(row["username"].as<string>(),
        row["password"].as<string>(),
        row["handle"].as<string>()
      );
  }
  return retval;    
}

} //namespace xblab