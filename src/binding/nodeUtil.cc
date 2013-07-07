#include <node_buffer.h>

#include "binding/nodeUtil.h"


using namespace v8;
using namespace node;
using namespace std;


namespace xblab {


extern Persistent<Function> xbNodeBufCtor;


Local<Value> NodeUtil::wrapBuf(const char *c, size_t len){
    v8::HandleScope scope;
    static const unsigned bufArgc = 3;

    Buffer *slowBuffer = Buffer::New(len);        
    memcpy(Buffer::Data(slowBuffer), c, len); // Buffer::Data = (void *)    
   
    Handle<Value> bufArgv[bufArgc] = { 
        slowBuffer->handle_,    // JS SlowBuffer handle
        Integer::New(len),      // SlowBuffer length
        Integer::New(0)         // Offset where "FastBuffer" should start
    };

    return scope.Close(xbNodeBufCtor->NewInstance(bufArgc, bufArgv));
}

} // namespace xblab