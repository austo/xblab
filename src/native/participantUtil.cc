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

  // Plaintext password will be compared with stored passhash on the server
  cred->set_password(baton->participant.password);
  cred->set_pub_key(baton->participant.publicKey);
  cred->set_group(baton->url);

  data->set_allocated_credential(cred);


  string sig, datastr;
  if (!data->SerializeToString(&datastr)) {
    throw util_exception("Failed to serialize broadcast data.");
  }
  try{
    sig = Crypto::sign(baton->participant.privateKey, datastr);
    trans.set_signature(sig);
    trans.set_allocated_data(data);
  }
  catch(crypto_exception& e){
    cout << "crypto exception: " << e.what() << endl;
  }
  // Protobuf-lite doesn't have SerializeToOstream
  stringstream plaintext, ciphertext;

  string pt;
  if (!trans.SerializeToString(&pt)){
    throw util_exception("Failed to serialize broadcast.");
  }

  plaintext << pt;

  Crypto::hybridEncrypt(plaintext, ciphertext);

  baton->xBuffer = ciphertext.str();
  baton->uvBuf.base = &baton->xBuffer[0];
  baton->uvBuf.len = baton->xBuffer.size();
}


// Mainly for consumption by the client, return Broadcast::Type
void
ParticipantUtil::digestBroadcast(ParticipantBaton *baton){
  
  Broadcast bc;
  if (!bc.ParseFromString(baton->xBuffer)){
    throw util_exception("Failed to deserialize broadcast.");
  }    

  string datastr;
  if (!bc.data().SerializeToString(&datastr)){
    throw util_exception("Failed to reserialize broadcast data.");
  }

  if (Crypto::verify(datastr, bc.signature())){
    const Broadcast::Data& data = bc.data();


    baton->returnNonce = string(data.nonce());

    switch(data.type()){
      case Broadcast::NEEDCRED: {
        baton->jsCallbackFactory = XbClient::requestCredentialFactory;
        baton->needsJsCallback = true;
        return;
      }
      case Broadcast::GROUPLIST:
      case Broadcast::GROUPENTRY:
      case Broadcast::BEGIN:
      case Broadcast::BROADCAST:
      case Broadcast::GROUPEXIT:
      case Broadcast::QUIT:
        throw util_exception("Broadcast type not implemented.");
    }
  }
  return;
}

} // namespace xblab
