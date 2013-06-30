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

// TODO: move to separate file
struct DataBaton {
    DataBaton(v8::Local<v8::Function> cb){
        request.data = this;
        callback = v8::Persistent<v8::Function>::New(cb);
    }
    ~DataBaton(){
        callback.Dispose();
    }
    uv_work_t request;
    std::string privateKeyFile;
    std::string password;
    std::string connstring;
    std::string buf;
    std::string nonce;
    std::string url;
    std::string err;
    void *auxData;
    v8::Persistent<v8::Function> callback;
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
 
        static std::string
        needCredBuf(std::string& privKeyFile, std::string& password, std::string& nonce);
        static void unpackMember(DataBaton* baton);

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