#ifndef MANAGER_H
#define MANAGER_H

#include <string>
#include <iostream>
#include <map>

#include <node.h>

#include "db.h"
#include "crypto.h"
#include "group.h"
#include "member.h"


namespace xblab {

class Manager : public node::ObjectWrap {

  public:
    Manager(std::string url) {
      try{
        // TODO: clean up calling convention
        Crypto::generateKey(this->priv_key_, this->pub_key_);
        group_ = Db::getGroup(url);
        
        // We've got the room ID, now get our members
        members_ = Db::getMembers(group_.id);
      }
       catch(std::exception& e){
        std::cout << "Exception caught: " << e.what() << std::endl;
        throw;
      }
    }

    ~Manager(){};


    static void Init(v8::Handle<v8::Object> module);
    static v8::Handle<v8::Value> NewInstance(const v8::Arguments& args);
    static v8::Handle<v8::Value> GetGroupName(v8::Local<v8::String>, const v8::AccessorInfo&);
    static void SetGroupName(v8::Local<v8::String>, v8::Local<v8::Value>, const v8::AccessorInfo&);
    static v8::Handle<v8::Value> SayHello(const v8::Arguments&);

    friend class Xblab;
    friend class Util;    
  

  private:
    static v8::Persistent<v8::Function> constructor;
    static v8::Persistent<v8::FunctionTemplate> ctor_template;
    static v8::Handle<v8::Value> New(const v8::Arguments& args);

    std::string encrypt(std::string);

    Group group_;
    std::map<int, Member> members_;
    std::string pub_key_;
    std::string priv_key_;
};
} //namespace xblab


#endif