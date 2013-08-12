#ifndef BATON_UTIL_H
#define BATON_UTIL_H

#include <string>
#include <map>

#include "common/util_exception.h"
#include "protobuf/xblab.pb.h"
#include "memberBaton.h"


namespace xblab {

// Message types - global to xblab namespace

enum MessageType {
  NEEDCRED,
  GROUPLIST,
  GROUPENTRY,
  BEGIN,
  BROADCAST,
  GROUPEXIT,
  QUIT,
  INVALID
};


class BatonUtil {
public:        
  friend class MemberBaton;   

private:
  BatonUtil(){ /* worker for baton */ };
  ~BatonUtil(){};

  static void
  needCredBuf(MemberBaton *baton);

  static void
  groupEntryBuf(MemberBaton *baton);

  static void
  startChatBuf(MemberBaton *baton);

  static void
  exceptionBuf(MemberBaton*, Broadcast::Type, std::string);

  static void
  exceptionBuf(MemberBaton*, Broadcast::Type, std::string, std::string);

  static void
  messageBuf(MemberBaton *baton);

  static void
  processTransmission(MemberBaton *baton); 
     
  static void
  processCredential(MemberBaton*, std::string& datastr,
    std::string signature, const Transmission::Credential& cred);

  static void
  processMessage(MemberBaton *baton, std::string& datastr,
    std::string signature, const Transmission::Payload& payload);

  static void
  routeTransmission(MemberBaton *baton,
    std::string& datastr, Transmission& trans);

  static void
  signData(Broadcast& bc, Broadcast::Data *data);

  static void
  signData(std::string privateKey, Broadcast& bc, Broadcast::Data *data);

  static void
  serializeToBuffer(MemberBaton *baton, Broadcast& bc);

  
};

} // namespace xblab


#endif