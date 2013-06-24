#include <vector>
#include <sstream>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <memory>
#include <botan/botan.h>
#include <botan/rsa.h>
#include <botan/look_pk.h>
#include "macros.h"
#include "manager.h"
#include "db.h"
#include "crypto.h"


using namespace std;
using namespace v8;
using namespace Botan;


namespace xblab {

Manager::Manager(string url) {
    try{

        // TODO: clean up calling convention
        Crypto::generateKey(this->priv_key_, this->pub_key_);
        group_ = Db::getGroup(url);
        
        // We've got the room ID, now get our members
        members_ = Db::getMembers(group_.id);
	 }
	 catch(exception& e){
		cout << "Exception caught: " << e.what() << endl;
        throw;
	 }
}

Persistent<Function> Manager::constructor;
Persistent<FunctionTemplate> Manager::ctor_template;

void Manager::Init(Handle<Object> module) {
	// Constructor function template

	Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
    ctor_template = Persistent<FunctionTemplate>::New(tpl);
    ctor_template->InstanceTemplate()->SetInternalFieldCount(1);
	ctor_template->SetClassName(String::NewSymbol("Manager"));
	//tpl->InstanceTemplate()->SetInternalFieldCount(1);    
	ctor_template->InstanceTemplate()->SetAccessor(String::New("groupName"),
        GetGroupName, SetGroupName);

	// Methods
	NODE_SET_PROTOTYPE_METHOD(ctor_template, "sayHello", SayHello);

	// Prototype    
	constructor = Persistent<Function>::New(ctor_template->GetFunction());

    module->Set(String::NewSymbol("Manager"), ctor_template->GetFunction());        

}

// args -> group url
// TODO: change to JS object ctor
Handle<Value> Manager::New(const Arguments& args) {
	HandleScope scope; 
    string url;
    if (!args[0]->IsUndefined()){
        String::Utf8Value temp_url(args[0]->ToString());
        url = string(*temp_url);
    }
    else{
        url = string("default");
    }

	Manager* instance = new Manager(url);
	
	instance->Wrap(args.This());
	return args.This();
}

Handle<Value> Manager::NewInstance(const Arguments& args) {
	HandleScope scope;
	const unsigned argc = 1;
	Handle<Value> argv[argc] = { args[0] };
	Local<Object> instance = constructor->NewInstance(argc, argv);
	return scope.Close(instance);
}

Handle<Value> Manager::GetGroupName(Local<String> property, const AccessorInfo& info) {
	// Extract C++ request object from JS wrapper
	Manager* instance = ObjectWrap::Unwrap<Manager>(info.Holder());
	return String::New(instance->group_.url.c_str());
}

void Manager::SetGroupName(Local<String> property, Local<Value> value, const AccessorInfo& info) {    
    THROW_FIELD_EX(property); //readonly
}

Handle<Value> Manager::SayHello(const Arguments& args) {
	HandleScope scope;
	Manager* instance = ObjectWrap::Unwrap<Manager>(args.This());	
	stringstream ss;
	ss << "Greetings from your manager and welcome to group " << instance->group_.name 
       << " at " << instance->group_.url << endl << "members:";
    map<int, Member>::const_iterator mitr = instance->members_.begin();
    for (; mitr != instance->members_.end(); ++mitr){
        ss << endl << mitr->second.handle;
    }

    string ct = instance->encrypt("super-sexxxy");

    Handle<Value> argv[2] = {
        String::New("decrypted"),
        String::New(ct.c_str())
    };

    node::MakeCallback(args.This(), "emit", 2, argv);

	return scope.Close(String::New(ss.str().c_str())); //String::New(ss.str().c_str()));
} 


// TODO: Encrypt should take member token, then encrypt with member public key
string Manager::encrypt(string in){
    string ciphertext = Crypto::hybridEncrypt(this->pub_key_, in);
    return Crypto::hybridDecrypt(this->priv_key_, ciphertext);
}
} // namespace xblab
