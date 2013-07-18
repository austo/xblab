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
  needCredBuf(MemberBaton* baton);

  static void
  groupEntryBuf(MemberBaton* baton);

  static void
  exceptionBuf(MemberBaton*, Broadcast::Type, std::string);

  static void
  initializeMember(MemberBaton* baton); 
     
  static void
  processCredential(MemberBaton*, std::string&,
    std::string, const Transmission::Credential&);
  
};

} // namespace xblab


#endif