#include <iostream>
#include <exception>

#include "manager.h"
#include "db.h"
#include "crypto.h"


using namespace std;

namespace xblab{

Manager::Manager(string url) {
    try{
        // TODO: clean up calling convention
        Crypto::generateKey(this->priv_key_, this->pub_key_);
        group_ = Db::getGroup(url);
        
        // We've got the room ID, now get our members
        members = Db::getMembers(group_.id);

        // Each member has a reference to the manager
        map<int, Member>::iterator mitr = members.begin();
        for (; mitr != members.end(); ++mitr){
            mitr->second.manager = this;
        }

    }
     catch(std::exception& e){
        cout << "Exception caught: " << e.what() << std::endl;
        throw;
    }
}

} // namespace xblab


