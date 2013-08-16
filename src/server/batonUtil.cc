#include <iostream>
#include <sstream>
#include <fstream> // TODO: add logging layer
#include <map>

#include <unistd.h>

#include <uv.h>

#include <assert.h>

#include "common/macros.h"
#include "common/common.h"
#include "common/logger.h"
#include "batonUtil.h"
#include "common/crypto.h"
#include "manager.h"
#include "member.h"
#include "db.h"
#include "db_exception.h"


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

  signData(bc, data);

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
  baton->nonce = Crypto::generateNonce();
  Broadcast bc;
  Broadcast::Data *data = new Broadcast::Data();
  Broadcast::Session *sess = new Broadcast::Session();

  // TODO: use member for this?
  data->set_type(Broadcast::GROUPENTRY);
  data->set_nonce(baton->nonce);
  data->set_return_nonce(baton->returnNonce);
  sess->set_pub_key(baton->member->manager->publicKey);

  data->set_allocated_session(sess);

  signData(bc, data);
  serializeToBuffer(baton, bc);
}


void
BatonUtil::setupBuf(MemberBaton *baton) {
  baton->nonce = Crypto::generateNonce();
  Broadcast bc;
  Broadcast::Data *data = new Broadcast::Data();
  Broadcast::Setup *setup = new Broadcast::Setup();

  data->set_type(Broadcast::SETUP);
  data->set_nonce(baton->nonce);
  data->set_return_nonce(baton->returnNonce);

  string sched = string(
    (char *)&baton->member->schedule[0],
    (baton->member->schedule.size() * sizeof(sched_t)));
  setup->set_schedule(sched);

  data->set_allocated_setup(setup);

  signData(baton->member->manager->getPrivateKey(), bc, data);
  serializeToBuffer(baton, bc);  
}


void
BatonUtil::startChatBuf(MemberBaton *baton) {
  baton->nonce = Crypto::generateNonce();
  Broadcast bc;
  Broadcast::Data *data = new Broadcast::Data();
  Broadcast::Prologue *prologue = new Broadcast::Prologue();

  // TODO: set manager->moduloCalculated_ false on round end
  prologue->set_modulo(baton->member->manager->getTargetModulo());

  data->set_type(Broadcast::BEGIN);
  data->set_nonce(baton->nonce);
  data->set_allocated_prologue(prologue);  

  signData(baton->member->manager->getPrivateKey(), bc, data);
  serializeToBuffer(baton, bc);
}


void
BatonUtil::exceptionBuf(
  MemberBaton* baton, Broadcast::Type type, std::string what){ 
  exceptionBuf(baton, type, what, baton->member->publicKey);
}


void
BatonUtil::exceptionBuf(
  MemberBaton* baton, Broadcast::Type type,
  std::string what, std::string pubkey){ 

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

  signData(bc, data);

  // TODO: serializeToBuffer overload for initial public key
  string retval;
  if (!bc.SerializeToString(&retval)) {
    throw util_exception("Failed to serialize broadcast.");
  }

  Crypto::hybridEncrypt(pubkey, retval);

  baton->xBuffer = retval;    
  baton->uvBuf.base = &baton->xBuffer[0];
  baton->uvBuf.len = baton->xBuffer.size();
}


void
BatonUtil::messageBuf(MemberBaton *baton) {
  baton->nonce = Crypto::generateNonce();
  Broadcast bc;
  Broadcast::Data *data = new Broadcast::Data();
  Broadcast::Payload *payload = new Broadcast::Payload();

  payload->set_modulo(baton->member->manager->getTargetModulo());
  payload->set_content(baton->member->manager->getRoundMessage());
  payload->set_is_important(baton->member->manager->roundIsImportant());

  data->set_type(Broadcast::BROADCAST);
  data->set_nonce(baton->nonce);
  data->set_allocated_payload(payload);  

  signData(baton->member->manager->getPrivateKey(), bc, data);
  serializeToBuffer(baton, bc);  
}


// Decrypt and compare nonces
void
BatonUtil::processTransmission(MemberBaton* baton) {
  string buf;
  // Use session key if chat membership has been established
  if (baton->hasMember() && baton->member->ready) {
    cout << "(" << baton->member->handle 
      << ") baton had member and member is ready\n";
    buf = baton->member->manager->decryptSessionMessage(baton->xBuffer);
  }
  else {
    buf = Crypto::hybridDecrypt(baton->xBuffer);
  }

  Transmission trans;
  if (!trans.ParseFromString(buf)) {
    throw util_exception("Failed to deserialize transmission.");
  }    

  string datastr;
  if (!trans.data().SerializeToString(&datastr)) {
    throw util_exception("Failed to reserialize transmission data.");
  }

  if (baton->nonce != trans.data().return_nonce()) {
    throw util_exception("Last nonce does not match transmission nonce.");
  }
  // Save incoming nonce for rebroadcast
  baton->returnNonce = string(trans.data().nonce());

  routeTransmission(baton, datastr, trans);
}


// Decide what to do based on transmission type
void
BatonUtil::routeTransmission(
  MemberBaton *baton, string& datastr, Transmission& trans) {
  switch (trans.data().type()) {
    case Transmission::CRED: {
      if (!baton->hasMember()) {
        processCredential(baton, datastr, trans.signature(),
          trans.data().credential());
      }
      return; 
    }
    case Transmission::ENTER: {
      cout << "ENTER message recieved from " << baton->member->handle << endl;
      // baton->getSetup();
      baton->member->ready = true;
      return;
    }
    case Transmission::READY: {
      cout << "READY message recieved from " << baton->member->handle << endl;
      baton->member->clientHasSchedule = true;    
      return;
    }
    case Transmission::TRANSMIT: {
      cout << "TRANSMIT recieved from " << baton->member->handle << endl;
      try {
        processMessage(
          baton, datastr, trans.signature(), trans.data().payload());
      }
      catch (util_exception& e) {
        baton->err = e.what();
      }
      return;
    }
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

  // NOTE: if uv_mutex_lock'ing here, macroize unlock and return

  typedef map<int, Member>::iterator memb_iter;

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

  Manager *mgr = NULL;

  if (xbManagers.find(baton->url) == xbManagers.end()) {
    // We want a "singleton" manager per group,
    // so check for group manager again inside the lock.
    uv_mutex_lock(&xbMutex);
    if (xbManagers.find(baton->url) == xbManagers.end()) {
      try {
        mgr = new Manager(baton->url);
        xbManagers.insert(pair<string, Manager*>(baton->url, mgr));
      }
      catch (db_exception& e) { // can't find group, close connection
        cout << rightnow() << e.what() << endl;
        baton->err = string(e.what());
        mgr = NULL;
      }
    }
    else {
      mgr = xbManagers.at(baton->url);
    }
    uv_mutex_unlock(&xbMutex);    
  }
  else {
    mgr = xbManagers.at(baton->url);
  }

  if (mgr == NULL) {
    exceptionBuf(baton, Broadcast::ERROR, baton->err, pubkey);
    return;
  }

  // cout << "public key for " << un << " before assume:\n" <<
  //   pubkey << endl;
  Member *m = new Member(un, pw, pubkey, true);

  memb_iter mitr = mgr->members.begin();
  for (; mitr != mgr->members.end(); ++mitr) {

    try {
      if (*m == mitr->second) {
        /* TODO: this may not be the best solution:
         * Handles case when client has disconnected and reconnected.
         * We may want to forbid this.
         */         
        baton->member = &mitr->second;
        baton->member->baton = baton;
        if (!mitr->second.present) {
          mitr->second.assume(m); // TODO make threadsafe and move to manager
          cout << rightnow() << mitr->second.username
             << " entered group " << mgr->group.url << endl <<
             /*"with public key:\n" << mitr->second.publicKey << */endl;
          baton->getGroupEntry();
          // TODO: check if all members have arrived and begin round
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
      cout << rightnow() << e.what();
      baton->err = e.what();
      delete m;
      exceptionBuf(baton, Broadcast::ERROR, e.what());
      
      return; 
    }
  }
  stringstream ss;
  ss << rightnow() << m->username << " is not a member of "
    << mgr->group.url << endl;
  delete m;
  exceptionBuf(baton, Broadcast::ERROR, ss.str());

  return;
}


void
BatonUtil::processMessage(MemberBaton *baton, string& datastr,
  string signature, const Transmission::Payload& payload) {  
  
  /* Can user broadcast?
   * If so, decrypt and verify message.
   * If message is okay, is the message important?
   * If so, set manager round message and broadcast with next round modulo.
   * If not, broadcast no message with next round modulo. 
   */
  uv_mutex_lock(&xbMutex);
  if (Manager::memberCanTransmit(baton->member->manager, baton->member)) {
    FileLogger logger;
    logger.setFile(logname());
    
    if (!Crypto::verify(baton->member->publicKey, datastr, signature)) {       
      F_LOG(ERROR) << "offending public key for " <<
        baton->member->handle << ":\n" <<
        baton->member->publicKey << endl <<
        "signature:\n" << signature << endl << 
        "msg:\n" << datastr;      
      throw util_exception("User key not verified.");
    }
    if (payload.is_important()) {
      baton->member->manager->setRoundMessage(payload.content());     
    }
    else {      
      F_LOG(DEBUG) <<
        baton->member->handle << " has nothing to say.";
    }
  }
  uv_mutex_unlock(&xbMutex);
}


void
BatonUtil::signData(Broadcast& bc, Broadcast::Data *data) {
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
}


void
BatonUtil::signData(
  string privateKey, Broadcast& bc, Broadcast::Data *data) {
  string sig, datastr;
  if (!data->SerializeToString(&datastr)) {
    throw util_exception("Failed to serialize broadcast data.");
  }
  try {
    sig = Crypto::sign(privateKey, datastr);
    bc.set_signature(sig);
    bc.set_allocated_data(data);
  }
  catch(crypto_exception& e) {
    cout << rightnow() << "crypto exception: " << e.what() << endl;
  }
}


void
BatonUtil::serializeToBuffer(MemberBaton *baton, Broadcast& bc) {
  string retval;
  if (!bc.SerializeToString(&retval)) {
    throw util_exception("Failed to serialize broadcast.");
  }

  Crypto::hybridEncrypt(baton->member->publicKey, retval);

  baton->xBuffer = retval;    
  baton->uvBuf.base = &baton->xBuffer[0];
  baton->uvBuf.len = baton->xBuffer.size();
}


} //namespace xblab