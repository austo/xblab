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
void Manager::Init() {
	// Constructor function template
	Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
	tpl->SetClassName(String::NewSymbol("Manager"));
	tpl->InstanceTemplate()->SetInternalFieldCount(1);    
	tpl->InstanceTemplate()->SetAccessor(String::New("groupName"), GetGroupName, SetGroupName);

	// Methods
	NODE_SET_PROTOTYPE_METHOD(tpl, "sayHello", SayHello);

	// Prototype    
	constructor = Persistent<Function>::New(tpl->GetFunction());
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
        ss<< endl << mitr->second.handle;
    }
    instance->Encrypt("super-sexxxy");
	string greeting = ss.str();
	return scope.Close(String::New(greeting.c_str()));
} 


// TODO: Encrypt should take member token, then encrypt with member public key
void Manager::Encrypt(string in){
    string ciphertext = Crypto::hybridEncrypt(this->pub_key_, in);
    // cout << "ciphertext: " << ciphertext << endl;

    string plaintext = Crypto::hybridDecrypt(this->priv_key_, ciphertext);

    cout << "resulting plaintext: " << plaintext << endl;
}
} // namespace xblab
