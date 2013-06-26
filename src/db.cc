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

    assert(group_id > 0);

    connection c(connectionString());
    map<int, Member> retval = map<int, Member>();
    work txn(c);
    stringstream ss;
    // No need to quote the string here as it's coming from previous call;
    ss << "select * from get_group_users(" << group_id << ");";
    cout << ss.str() << endl;
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