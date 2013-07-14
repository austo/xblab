#ifndef NATIVE_UTIL_H
#define NATIVE_UTIL_H

#include <string>
#include <map>

#include "util_exception.h"
#include "protobuf/xblab.pb.h"
#include "clientBaton.h"


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


class Util {
public:        
  friend class ClientBaton;   

private:
  Util(){ /* worker for baton */ };
  ~Util(){};

  static void
  needCredBuf(ClientBaton* baton);

  static void
  groupEntryBuf(ClientBaton* baton);

  static void
  exceptionBuf(ClientBaton*, Broadcast::Type, std::string);

  static void
  initializeMember(ClientBaton* baton); 
     
  static void
  processCredential(ClientBaton*, std::string&,
    std::string, const Transmission::Credential&);
};

} // namespace xblab


#endif