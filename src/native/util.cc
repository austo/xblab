#include <iostream>
#include <sstream>
#include <vector>
#include <map>

#include "macros.h"
#include "util.h"
#include "crypto.h"
#include "manager.h"
#include "member.h"
#include "db.h"

#define DL_EX_PREFIX "Util: "

using namespace std;

namespace xblab {


extern string xbConnectionString;
extern string xbPublicKeyFile;
extern string xbPrivateKeyFile;
extern string xbKeyPassword;

extern map<string, Manager*> xbManagers;


// TODO: take DataBaton
string
Util::needCredBuf(string& nonce){ 
    // cout << "generating nonce\n";   
    nonce = Crypto::generateNonce();
    // cout << "have nonce\n";
    Broadcast bc;
    Broadcast::Data *data = new Broadcast::Data();
    // cout << "have data\n";

    data->set_type(Broadcast::NEEDCRED);
    data->set_nonce(nonce);

    string sig, datastr;
    if (!data->SerializeToString(&datastr)) {
        throw util_exception("Failed to serialize broadcast data.");
    }
    try{
        // cout << "signing message\n";
        // cout << privKeyFile << endl << password << endl;
        sig = Crypto::sign(datastr);
        // cout << "message signed\n";
        bc.set_signature(sig);
        // cout << "signature set\n";
        bc.set_allocated_data(data); //bc now owns data - no need to free
        // cout << "data allocated\n";
    }
    catch(crypto_exception& e){
        cout << "crypto exception: " << e.what() << endl;
    }

    string retval;
    if (!bc.SerializeToString(&retval)){
        throw util_exception("Failed to serialize broadcast.");
    }

    return retval;
}


void
Util::unpackMember(ClientBaton* baton){
    string buf = Crypto::hybridDecrypt(baton->xBuffer);

    //TODO: switch on transmission type
    Transmission trans;
    if (!trans.ParseFromString(buf)){
        throw util_exception("Failed to deserialize transmission.");
    }    

    string datastr;
    if (!trans.data().SerializeToString(&datastr)){
        throw util_exception("Failed to reserialize transmission data.");
    }

    const Transmission::Data& data = trans.data();

    string retNonce(data.return_nonce());    

    if (baton->nonce == retNonce){

        // TODO: refactor, change to switch
        if (data.type() == Transmission::CRED){
            const Transmission::Credential& cred = data.credential();
            string pubkey(cred.pub_key());

            if (!Crypto::verify(pubkey, datastr, trans.signature())) { 
                throw util_exception("User key not verified.");
            }

            cout << "Parse transmission: user signature verified.\n";
            string un(cred.username());
            string pw(cred.password());
            baton->url = string(cred.group());

            Manager *mgr;

            if (xbManagers.find(baton->url) == xbManagers.end()){
                // add new manager (to be wrapped later)
                mgr = new Manager(baton->url);
                xbManagers.insert(pair<string, Manager*>(baton->url, mgr));
            }
            else {
                mgr = xbManagers.at(baton->url);
            }

            Member *m = new Member(un, pw, pubkey, true);

            map<int, Member>::iterator mitr = mgr->members.begin();
            for (; mitr != mgr->members.end(); ++mitr){

                try {
                    if (*m == mitr->second){
                        mitr->second.assume(m);
                        cout << "member " << mitr->second.username
                             << " assumed with pub_key:\n" 
                             << mitr->second.public_key << endl;
                        baton->member = &mitr->second;
                        return; 
                    }
                }
                catch (exception& e){
                    cout << e.what();
                    baton->err = e.what();
                }
            }
            baton->err = "Unable to find member";
        }
    }    
    else{
        throw util_exception("Last nonce does not match transmission nonce.");
    }
}

} //namespace xblab

