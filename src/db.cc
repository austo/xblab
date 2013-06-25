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

string Db::connectionString(){
    static string retval = string(*(v8::String::Utf8Value(connstring)));
    return retval;
}

Group Db::getGroup(string url){ //Calling code responsible for string trimming 
    connection c(connectionString());
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

User Db::getUnattachedUser(std::string& username, std::string& password){
    connection c(connectionString());
    User user;
    work txn(c);

    stringstream qss;
    qss << "select * from get_user_groups(" << txn.quote(username)
        << "," << txn.quote(password) << " );";
    result flat_user = txn.exec(qss.str());

    if (flat_user.size() == 0){
        throw db_exception("User not found.");
    }
    else{
        user.id = flat_user[0][0].as<int>();
        user.username = string(username);
        user.password = string(password);
        user.groups = map<int, Group>();

        result::const_iterator row = flat_user.begin();
        for (; row != flat_user.end(); ++row){
            user.groups.insert(pair<int, Group>(
                row["id"].as<int>(),
                Group(row["id"].as<int>(),
                    row["name"].as<string>(),
                    row["url"].as<string>())));            
        }
    }
    return user;
}


map<int, Member> Db::getMembers(int group_id){
    connection c(connectionString());
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