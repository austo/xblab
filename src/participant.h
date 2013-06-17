#ifndef __PARTICIPANT_H
#define __PARTICIPANT_H

#include <string>
#include <node.h>

namespace xblab {

class Participant : public node::ObjectWrap {

    public:
        Participant();
        ~Participant(){};

        static void Init();
        static v8::Handle<v8::Value> NewInstance(const v8::Arguments& args);

        //v8 property accessors
        static v8::Handle<v8::Value> GetHandle(v8::Local<v8::String>, const v8::AccessorInfo&);
        static void SetHandle(v8::Local<v8::String>, v8::Local<v8::Value>, const v8::AccessorInfo&);

           
        
        //TODO: Sign
        //TODO: Decrypt


    private:
        Participant(std::string username, std::string password, std::string group);
        static v8::Persistent<v8::Function> constructor;
        static v8::Handle<v8::Value> New(const v8::Arguments& args);
        void BuildPacket(std::string);
        void DigestPacket(std::string);

        std::string username_;
        std::string password_;
        std::string handle_;
        std::string group_;
        std::string pub_key_;
        std::string priv_key_;
        std::string round_nonce_; //may need to rethink this 
        int round_modulus_;    
};

} //namespace xblab

#endif