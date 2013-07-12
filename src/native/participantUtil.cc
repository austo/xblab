#include <iostream>
#include <sstream>
#include <vector>

#include "macros.h"
#include "crypto.h"


extern string xbPublicKeyFile;

string
Util::packageParticipantCredentials(ParticipantBaton *baton){
  baton->nonce = Crypto::generateNonce();
  
  Transmission trans;
  Transmission::Data *data = new Transmission::Data();
  Transmission::Credential *cred = new Transmission::Credential();

  data->set_type(Transmission::CRED);
  data->set_nonce(nonce);
  data->set_return_nonce(client->return_nonce_);

  cred->set_username(client->username_);

  // Plaintext password will be compared with stored passhash on the server
  cred->set_password(client->password_);
  cred->set_group(client->group_);
  cred->set_pub_key(client->pub_key_);

  data->set_allocated_credential(cred);


  string sig, datastr;
  if (!data->SerializeToString(&datastr)) {
    throw util_exception("Failed to serialize broadcast data.");
  }
  try{
    sig = Crypto::sign(client->priv_key_, datastr);
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

  return ciphertext.str();
}


// Mainly for consumption by the client, return Broadcast::Type
MessageType
Util::parseNeedCredential(ParticipantBaton *baton){

  baton->stringifyBuffer();
  MessageType retval = INVALID;
  //TODO: switch on broadcast type
  Broadcast bc;
  if (!bc.ParseFromString(baton->xbuffer)){
    throw util_exception("Failed to deserialize broadcast.");
  }    

  string datastr;
  if (!bc.data().SerializeToString(&datastr)){
    throw util_exception("Failed to reserialize broadcast data.");
  }

  if (Crypto::verify(datastr, bc.signature())){
    const Broadcast::Data& data = bc.data();

    baton->return_nonce_ = string(data.nonce());

    // TODO: no casts - proper type interrogation
    retval = (MessageType) data.type();
  }       

  return retval;
}