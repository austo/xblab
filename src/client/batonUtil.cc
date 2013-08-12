#include <iostream>
#include <sstream>

#include "common/macros.h"
#include "common/common.h"
#include "common/crypto.h"
#include "client/batonUtil.h"
#include "client/binding/xbClient.h"
#include "client/client.h"

using namespace std;

namespace xblab {

extern string xbPublicKeyFile;

void
BatonUtil::packageCredential(MemberBaton *baton) {
  baton->nonce = Crypto::generateNonce();
  
  Transmission trans;
  Transmission::Data *data = new Transmission::Data();
  Transmission::Credential *cred = new Transmission::Credential();

  data->set_type(Transmission::CRED);
  data->set_nonce(baton->nonce);
  data->set_return_nonce(baton->returnNonce);

  cred->set_username(baton->member.username);

  // Plaintext user password will be compared to stored passhash
  cred->set_password(baton->member.password);
  cred->set_pub_key(baton->member.publicKey);
  cred->set_group(baton->url);

  data->set_allocated_credential(cred);

  signData(baton->member.privateKey, trans, data);
  serializeToBuffer(baton, trans);  
}


void
BatonUtil::digestBroadcast(MemberBaton *baton) {
  
  Broadcast bc;
  if (!bc.ParseFromString(baton->xBuffer)){
    string plaintext = Crypto::hybridDecrypt(
      baton->member.privateKey, baton->xBuffer);
    if (!bc.ParseFromString(plaintext)) {
      throw util_exception("Failed to deserialize broadcast.");
    }
  }    

  string datastr;
  if (!bc.data().SerializeToString(&datastr)) {
    throw util_exception("Failed to reserialize broadcast data.");
  }

  if (verifySignature(baton, datastr, bc.signature())) {
    const Broadcast::Data& data = bc.data();

    baton->returnNonce = string(data.nonce());
    baton->nonce = Crypto::generateNonce();
    
    switch(data.type()){
      case Broadcast::NEEDCRED: {
        baton->jsCallbackFactory = XbClient::requestCredentialFactory;
        baton->needsJsCallback = true;
        return;
      }
      case Broadcast::GROUPENTRY: {
        // Store key && emit "welcome" event
        enterGroup(baton, data);
        return;
      }
      case Broadcast::ERROR: {
        baton->err = data.error().what();
        return;
      }
      case Broadcast::NO_OP: {
        baton->err = data.no_op().what();
        return;
      }
      case Broadcast::BEGIN: {
        cout << rightnow() << "received BEGIN\n";
        startChat(baton, data);
        return;
      }


      case Broadcast::GROUPLIST:
      case Broadcast::BROADCAST:
      case Broadcast::GROUPEXIT:
      case Broadcast::QUIT: {
        throw util_exception("Broadcast type not implemented.");
      }
    }
  }
  return;
}


void
BatonUtil::enterGroup(
  MemberBaton *baton, const Broadcast::Data& data) {
  const Broadcast::Session& session = data.session();
  baton->member.sessionKey = session.pub_key();

  string sched(session.schedule());
  baton->member.schedule = vectorize_string<sched_t>(sched);

  // uv_write "READY" message & sign with xbPublicKeyFile
  chatReady(baton);

#ifdef TRACE
  for (int i = 0, n = baton->member.schedule.size(); i < n; ++i) {
    cout << baton->member.schedule[i] << ", ";
  }
  cout << endl;
#endif

  baton->jsCallbackFactory = XbClient::groupEntryFactory;
  baton->needsJsCallback = true;
}


void
BatonUtil::chatReady(MemberBaton *baton) {

  Transmission trans;
  Transmission::Data *data = new Transmission::Data();
  data->set_type(Transmission::READY);
  data->set_nonce(baton->nonce);
  data->set_return_nonce(baton->returnNonce);

  signData(baton->member.privateKey, trans, data);

  // Ready message is sent using original key, whereupon
  // we switch to session key for all subsequent messages.
  serializeToBuffer(baton, trans);

  baton->needsUvWrite = true;
  baton->member.ready = true;
}


void
BatonUtil::startChat(
  MemberBaton *baton, const Broadcast::Data& data) {
#ifdef TRACE
  cout << "inside start chat\n";
#endif
  const Broadcast::Prologue& prologue = data.prologue();
#ifdef TRACE
  cout << "got prologue\n";
#endif
  baton->member.modulo = prologue.modulo();
  cout << baton->member.username << ": " << baton->member.modulo << endl;
  baton->jsCallbackFactory = XbClient::startChatFactory;
  baton->needsJsCallback = true;
  packageTransmission(baton);
}


void
BatonUtil::packageTransmission(MemberBaton *baton) {
  Transmission trans;
  Transmission::Data *data = new Transmission::Data();
  data->set_type(Transmission::TRANSMIT);
  data->set_nonce(baton->nonce);
  data->set_return_nonce(baton->returnNonce);

  Transmission::Payload *payload = new Transmission::Payload();

  if (baton->member.canTransmit()) {
    payload->set_is_important(baton->member.hasMessage);    

    if (baton->member.hasMessage) {
      payload->set_content(baton->member.message);
    }
    else {
      cout << "setting content to random message.\n";
      payload->set_content(
        Crypto::generateRandomMessage(XBMAXMESSAGELENGTH));
    }
  }
  else {
    payload->set_is_important(false);
    payload->set_content(Crypto::generateRandomMessage(XBMAXMESSAGELENGTH));
  }

  data->set_allocated_payload(payload);
  signData(baton->member.privateKey, trans, data);
  serializeToBuffer(baton, trans);
  baton->needsUvWrite = true;
}


void
BatonUtil::signData(
  string& privateKey, Transmission& trans, Transmission::Data *data) {
  string sigstr, datastr;
  if (!data->SerializeToString(&datastr)) {
    throw util_exception("Failed to serialize broadcast data.");
  }
  try{
    sigstr = Crypto::sign(privateKey, datastr);
    trans.set_signature(sigstr);
    trans.set_allocated_data(data);
  }
  catch(crypto_exception& e){
    cout << "crypto exception: " << e.what() << endl;
  }
}


void
BatonUtil::serializeToBuffer(
  MemberBaton *baton, Transmission& trans) {

  stringstream plaintext, ciphertext;
  string ptstr;
  if (!trans.SerializeToString(&ptstr)){
    throw util_exception("Failed to serialize broadcast.");
  }

  plaintext << ptstr;
  if (baton->member.ready) {
  #ifdef DEBUG
    cout << "serializeToBuffer - using session key\n";
  #endif
    Crypto::hybridEncrypt(baton->member.sessionKey, plaintext, ciphertext);
  }
  else {
    Crypto::hybridEncrypt(plaintext, ciphertext);
  }

  baton->xBuffer = ciphertext.str();
  baton->uvBuf.base = &baton->xBuffer[0];
  baton->uvBuf.len = baton->xBuffer.size();
}


// Use session key or global public key, depending on chat state.
bool
BatonUtil::verifySignature(
  MemberBaton *baton, string& datastr, string signature) {
  return baton->member.ready ?
    Crypto::verify(baton->member.sessionKey, datastr, signature) :
    Crypto::verify(datastr, signature);
}

} // namespace xblab