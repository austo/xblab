#include <cstdlib>
#include <iostream>
#include <exception>

#include "manager.h"
#include "db.h"
#include "db_exception.h"
#include "common/crypto.h"
#include "common/macros.h"


using namespace std;

namespace xblab{

Manager::Manager(string url) {
  Crypto::generateKey(this->privateKey_, this->publicKey);
  group = Db::getGroup(url);
  
  // We've got the room ID, now get our members
  members = Db::getMembers(group.id);
  seed = Crypto::generateRandomInt();

  // Each member has a reference to the manager
  map<int, Member>::iterator mitr = members.begin();
  for (; mitr != members.end(); ++mitr){
    mitr->second.manager = this;
  }

  nMembers_ = members.size();
  roundModulii_ = (int *)malloc(sizeof(int) * nMembers_);
  currentRound_ = 0;
  cout << rightnow() << "Manager created for group "
      << group.url << " (\"" << group.name << "\")" << endl;  
}

Manager::~Manager(){
  free(roundModulii_);
}

// TODO: round schedule

} // namespace xblab


