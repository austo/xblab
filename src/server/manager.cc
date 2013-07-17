#include <cstdlib>
#include <iostream>
#include <exception>

#include "manager.h"
#include "db.h"
#include "common/crypto.h"
#include "common/macros.h"


using namespace std;

namespace xblab{

Manager::Manager(string url) {
  try{
    Crypto::generateKey(this->privateKey_, this->publicKey);
    group = Db::getGroup(url);
    
    // We've got the room ID, now get our members
    members = Db::getMembers(group.id);

    // Each member has a reference to the manager
    map<int, Member>::iterator mitr = members.begin();
    for (; mitr != members.end(); ++mitr){
      mitr->second.manager = this;
      // set seed
      mitr->second.seed = Crypto::generateRandomInt();
    }

    nMembers_ = members.size();
    roundModulii_ = (int *)malloc(sizeof(int) * nMembers_);
    currentRound_ = 0;
  }
  catch(std::exception& e){
    cout << "Exception caught: " << e.what() << std::endl;
    throw;
  }
}

Manager::~Manager(){
  free(roundModulii_);
}



// TODO: round schedule

} // namespace xblab


