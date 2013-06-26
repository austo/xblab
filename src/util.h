#ifndef UTIL_H
#define UTIL_H

#include <string>
#include <map>
#include <node.h>
#include <exception>
#include "group.h"
#include "member.h"
#include "protobuf/xblab.pb.h"


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
        Util();
        ~Util();
        
        #ifndef XBLAB_CLIENT
 
        static std::string needCredBuf(std::string& nonce);
        static void parseTransmission(std::string lastNonce,
            std::string& buf, std::map<std::string, v8::Handle<v8::Object> >& managers);

        #endif
        
        static MessageType parseBroadcast(std::string& in, void* out);
        static std::string packageParticipantCredentials(void* data);

        static v8::Local<v8::Value> wrapBuf(const char *c, size_t len);

        static std::string v8ToString(v8::Local<v8::Value> value) {
            v8::String::Utf8Value utf8Value(value);
            return std::string(*utf8Value);
        }

    private:
        
};

} //namespace xblab


#endif