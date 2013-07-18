#include <iostream>
#include <sstream>
#include <vector>
#include <map>

#include <unistd.h>

#include <uv.h>

#include <assert.h>

#include "common/macros.h"
#include "batonUtil.h"
#include "common/crypto.h"
#include "manager.h"
#include "member.h"
#include "db.h"

#define DL_EX_PREFIX "Util: "

using namespace std;

namespace xblab {


extern string xbConnectionString;
extern string xbPublicKeyFile;
extern string xbPrivateKeyFile;
extern string xbKeyPassword;

extern map<string, Manager*> xbManagers;

extern uv_mutex_t xbMutex;

void
BatonUtil::needCredBuf(MemberBaton* baton) { 
  baton->nonce = Crypto::generateNonce();
  Broadcast bc;
  Broadcast::Data *data = new Broadcast::Data();

  data->set_type(Broadcast::NEEDCRED);
  data->set_nonce(baton->nonce);

  string sig, datastr;
  if (!data->SerializeToString(&datastr)) {
    throw util_exception("Failed to serialize broadcast data.");
  }
  try{
    sig = Crypto::sign(datastr);
    bc.set_signature(sig);
    bc.set_allocated_data(data); //bc now owns data - no need to free
  }
  catch(crypto_exception& e) {
    cout << rightnow() << "crypto exception: " << e.what() << endl;
  }

  string retval;
  if (!bc.SerializeToString(&retval)) {
    throw util_exception("Failed to serialize broadcast.");
  }

  baton->xBuffer = retval;    
  baton->uvBuf.base = &baton->xBuffer[0];
  baton->uvBuf.len = baton->xBuffer.size();
}


void
BatonUtil::groupEntryBuf(MemberBaton* baton) { 

  // TODO: write group entry buffer
  baton->nonce = Crypto::generateNonce();
  Broadcast bc;
  Broadcast::Data *data = new Broadcast::Data();
  Broadcast::Session *sess = new Broadcast::Session();

  // TODO: use member for this?
  data->set_type(Broadcast::GROUPENTRY);
  data->set_nonce(baton->nonce);
  data->set_return_nonce(baton->returnNonce);
  sess->set_pub_key(baton->member->manager->publicKey);
  sess->set_seed(baton->member->seed);

  data->set_allocated_session(sess);

  string sig, datastr;
  if (!data->SerializeToString(&datastr)) {
    throw util_exception("Failed to serialize broadcast data.");
  }
  try {
    sig = Crypto::sign(datastr);
    bc.set_signature(sig);
    bc.set_allocated_data(data);
  }
  catch(crypto_exception& e) {
    cout << rightnow() << "crypto exception: " << e.what() << endl;
  }

  string retval;
  if (!bc.SerializeToString(&retval)) {
    throw util_exception("Failed to serialize broadcast.");
  }

  Crypto::hybridEncrypt(baton->member->publicKey, retval);

  baton->xBuffer = retval;    
  baton->uvBuf.base = &baton->xBuffer[0];
  baton->uvBuf.len = baton->xBuffer.size();
}


void
BatonUtil::exceptionBuf(
  MemberBaton* baton, Broadcast::Type type, std::string what){ 

  // TODO: write group entry buffer
  baton->nonce = Crypto::generateNonce();
  Broadcast bc;
  Broadcast::Data *data = new Broadcast::Data();

  switch (type) {
    case Broadcast::NO_OP: {
      Broadcast::No_Op *no_op = new Broadcast::No_Op();
      no_op->set_what(what);
      data->set_allocated_no_op(no_op);
      break;
    }
    case Broadcast::ERROR: {
      Broadcast::Error *error = new Broadcast::Error();
      error->set_what(what);
      data->set_allocated_error(error);
      break;
    }
    default:
      throw util_exception(
        "BatonUtil::exceptionBuf requires NO_OP or ERROR types.");
  }

  data->set_type(type);
  data->set_nonce(baton->nonce);
  data->set_return_nonce(baton->returnNonce);

  string sigstr, datastr;
  if (!data->SerializeToString(&datastr)) {
    throw util_exception("Failed to serialize broadcast data.");
  }
  try {
    sigstr = Crypto::sign(datastr);
    bc.set_signature(sigstr);
    bc.set_allocated_data(data);
  }
  catch(crypto_exception& e) {
    cout << rightnow() <<  "crypto exception: " << e.what() << endl;
  }

  string retval;
  if (!bc.SerializeToString(&retval)) {
    throw util_exception("Failed to serialize broadcast.");
  }

  Crypto::hybridEncrypt(baton->member->publicKey, retval);

  baton->xBuffer = retval;    
  baton->uvBuf.base = &baton->xBuffer[0];
  baton->uvBuf.len = baton->xBuffer.size();
}


void
BatonUtil::initializeMember(MemberBaton* baton) {
  string buf = Crypto::hybridDecrypt(baton->xBuffer);

  //TODO: switch on transmission type
  Transmission trans;
  if (!trans.ParseFromString(buf)) {
    throw util_exception("Failed to deserialize transmission.");
  }    

  string datastr;
  if (!trans.data().SerializeToString(&datastr)) {
    throw util_exception("Failed to reserialize transmission data.");
  }

  const Transmission::Data& data = trans.data();
  string retNonce(data.return_nonce());

  if (baton->nonce != retNonce) {
    throw util_exception("Last nonce does not match transmission nonce.");
  }
  // save incoming nonce for rebroadcast
  baton->returnNonce = string(data.nonce());

  // TODO: refactor into general "process transmission" method
  switch (data.type()) {
    case Transmission::CRED: {
      const Transmission::Credential& cred = data.credential();
      processCredential(baton, datastr, trans.signature(), cred); 
      return; 
    }
    case Transmission::ENTER:
    case Transmission::TRANSMIT:
    case Transmission::EXIT:
    case Transmission::QUIT:
    case Transmission::ERROR:
    case Transmission::NO_OP: {
      throw util_exception("Transmission not implemented.");
    }
  }
}


void
BatonUtil::processCredential(MemberBaton *baton, string& datastr,
  string signature, const Transmission::Credential& cred) {
  string pubkey(cred.pub_key());

  if (!Crypto::verify(pubkey, datastr, signature)) { 
    throw util_exception("User key not verified.");
  }
#ifdef DEBUG
  cout << rightnow() << "Process credential: user signature verified.\n";
#endif
  string un(cred.username());
  string pw(cred.password());
  baton->url = string(cred.group());

  Manager *mgr;

  if (xbManagers.find(baton->url) == xbManagers.end()) {
    // We want a "singleton" manager per group,
    // so check for group manager again inside the lock
    uv_mutex_lock(&xbMutex);
    if (xbManagers.find(baton->url) == xbManagers.end()) {
      mgr = new Manager(baton->url);
      xbManagers.insert(pair<string, Manager*>(baton->url, mgr));
    }
    else {
      mgr = xbManagers.at(baton->url);
    }
    uv_mutex_unlock(&xbMutex);    
  }
  else {
    mgr = xbManagers.at(baton->url);
  }

  Member *m = new Member(un, pw, pubkey, true);

  map<int, Member>::iterator mitr = mgr->members.begin();
  for (; mitr != mgr->members.end(); ++mitr) {

    try {
      if (*m == mitr->second) {
        /* 
         * TODO: this may not be the best solution:
         * Handles case when client has disconnected and reconnected.
         * We may want to forbid this.
         */
        baton->member = &mitr->second;
        if (!mitr->second.present) {
          mitr->second.assume(m);
          cout << rightnow() << mitr->second.username
             << " entered group " << mgr->group.url << endl; 
          baton->getGroupEntry();
        }
        else {
          stringstream ss;
          ss << rightnow() << mitr->second.username
             << " already present in " << mgr->group.url << endl;
          cout << ss.str();
          exceptionBuf(baton, Broadcast::NO_OP, ss.str());
        }
        return; 
      }      
    }
    catch (exception& e) {
      cout << e.what();
      baton->err = e.what();
      // TODO: Error
    }
  }
  stringstream ss;
  ss << rightnow() << m->username << " is not a member of "
    << mgr->group.url << endl;
  delete m;
  exceptionBuf(baton, Broadcast::ERROR, ss.str());
  return;
}


} //namespace xblab

