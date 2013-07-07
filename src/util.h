#ifndef UTIL_H
#define UTIL_H

#include <string>
#include <map>
#include <exception>
#include "protobuf/xblab.pb.h"
#include "baton.h"


namespace xblab {

// Message types enum - global to xblab namespace

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



class util_exception : public std::exception {
public:
    util_exception(){
        message_ = std::string("Data integrity exception.");
    }
    util_exception(std::string err_msg){
        message_ = err_msg;
    }
    ~util_exception() throw(){};
    virtual const char* what() const throw(){
        return this->message_.c_str();
    }
private:
    std::string message_;        
};

class Util {

    public:     
        
        #ifndef XBLAB_CLIENT 
        
        static std::string needCredBuf(std::string& nonce);
        static void unpackMember(DataBaton* baton);

        #endif
        
        static MessageType parseBroadcast(std::string& in, void* out);
        static std::string packageParticipantCredentials(void* data);

    private:
        Util(){ /* keep as "static" class */ };
        ~Util(){};  
};

} //namespace xblab


#endif