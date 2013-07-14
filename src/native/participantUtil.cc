#include <iostream>
#include <sstream>
#include <vector>

#include "macros.h"
#include "crypto.h"
#include "native/participantUtil.h"
#include "binding/xbClient.h"

using namespace std;

namespace xblab {


extern string xbPublicKeyFile;

void
ParticipantUtil::packageCredential(ParticipantBaton *baton){
  baton->nonce = Crypto::generateNonce();
  
  Transmission trans;
  Transmission::Data *data = new Transmission::Data();
  Transmission::Credential *cred = new Transmission::Credential();

  data->set_type(Transmission::CRED);
  data->set_nonce(baton->nonce);
  data->set_return_nonce(baton->returnNonce);

  cred->set_username(baton->participant.username);

  // Plaintext user password will be compared to stored passhash
  cred->set_password(baton->participant.password);
  cred->set_pub_key(baton->participant.publicKey);
  cred->set_group(baton->url);

  data->set_allocated_credential(cred);


  string sigstr, datastr;
  if (!data->SerializeToString(&datastr)) {
    throw util_exception("Failed to serialize broadcast data.");
  }
  try{
    sigstr = Crypto::sign(baton->participant.privateKey, datastr);
    trans.set_signature(sigstr);
    trans.set_allocated_data(data);
  }
  catch(crypto_exception& e){
    cout << "crypto exception: " << e.what() << endl;
  }
  // Protobuf-lite doesn't support SerializeToOstream
  stringstream plaintext, ciphertext;

  string ptstr;
  if (!trans.SerializeToString(&ptstr)){
    throw util_exception("Failed to serialize broadcast.");
  }

  plaintext << ptstr;
  Crypto::hybridEncrypt(plaintext, ciphertext);

  baton->xBuffer = ciphertext.str();
  baton->uvBuf.base = &baton->xBuffer[0];
  baton->uvBuf.len = baton->xBuffer.size();
}


void
ParticipantUtil::digestBroadcast(ParticipantBaton *baton){
  
  Broadcast bc;
  if (!bc.ParseFromString(baton->xBuffer)){
    string plaintext = Crypto::hybridDecrypt(
      baton->participant.privateKey, baton->xBuffer);
    if (!bc.ParseFromString(plaintext)) {
      throw util_exception("Failed to deserialize broadcast.");
    }
  }    

  string datastr;
  if (!bc.data().SerializeToString(&datastr)){
    throw util_exception("Failed to reserialize broadcast data.");
  }

  if (Crypto::verify(datastr, bc.signature())){
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
        // store key && emit "welcome" event
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

      case Broadcast::GROUPLIST:
      case Broadcast::BEGIN:
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
ParticipantUtil::enterGroup(
  ParticipantBaton *baton, const Broadcast::Data& data){
  const Broadcast::Session& session = data.session();
  baton->participant.sessionKey = session.pub_key();
  baton->participant.seed = session.seed();
  baton->jsCallbackFactory = XbClient::groupEntryFactory;
  baton->needsJsCallback = true;
}


} // namespace xblab
