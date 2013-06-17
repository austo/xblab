#include "db.h"
#include <iostream>
#include <sstream>
#include <pqxx/pqxx>
#include "macros.h"

using namespace std;
using namespace pqxx;


namespace xblab {

extern v8::Persistent<v8::String> connstring;

Db::Db(){ /* The goal is to keep this a "static" class */ }
Db::~Db(){}

string Db::conn_str(){
    static string retval = string(*(v8::String::Utf8Value(connstring->ToString())));
    return retval;
}

Group Db::get_group(string url){ //Calling code responsible for string trimming 
    connection c(conn_str());
    Group group;
    work txn(c);
    stringstream ss;
    ss << "select id, name from groups where url = " << txn.quote(url) << ";";
    result groups = txn.exec(ss.str());

    if (groups.size() > 1){
        throw db_exception("Multiple users found.");
    }
    else if (groups.size() == 0){
        throw db_exception("User not found.");
    }
    else{
        group.id = groups[0][0].as<int>();
        group.name = groups[0][1].as<string>();
        group.url = url;
    }
    return group;
}

map<int, Member> Db::get_members(int group_id){
    connection c(conn_str());
    map<int, Member> retval = map<int, Member>();
    work txn(c);
    stringstream ss;
    ss << "select  * from get_group_users(" << txn.quote(group_id) << ");";
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