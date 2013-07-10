#include <iostream>
#include <exception>

#include "manager.h"
#include "db.h"
#include "crypto.h"


using namespace std;

namespace xblab{

Manager::Manager(string url) {
    try{
        Crypto::generateKey(this->priv_key_, this->pub_key);
        group_ = Db::getGroup(url);
        
        // We've got the room ID, now get our members
        members = Db::getMembers(group_.id);

        // Each member has a reference to the manager
        map<int, Member>::iterator mitr = members.begin();
        for (; mitr != members.end(); ++mitr){
            mitr->second.manager = this;
            // set seed
            mitr->second.seed = Crypto::generateRandomInt();
        }
    }
     catch(std::exception& e){
        cout << "Exception caught: " << e.what() << std::endl;
        throw;
    }
}

} // namespace xblab


