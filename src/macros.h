#ifndef MACROS_H
#define MACROS_H

#include "util.h"

#define BITSIZE 2048
#define NONCE_SIZE 128
#define SIG_BUF_SIZE 4096
#define BCRYPT_WORK_FACTOR 12

#define SHA1 "EMSA1(SHA-1)"
#define SHA256 "EME1(SHA-256)"


// HACK - there may be a much nicer way to do this
#define THROW_FIELD_EX(prop) { stringstream ss;                 \
            ss << "Unable to set value of readonly field \'"    \
            << Util::v8ToString(prop) << "\'";                  \
            ThrowException(String::New(ss.str().c_str())); }

// Get node::Buffer constructor from JS land
#define XB_NODE_BUFFER_CTOR Persistent<Function>::New(Local<Function>::                \
            Cast(Context::GetCurrent()->Global()->Get(String::New("Buffer"))));     

#define THROW(prop) ThrowException(String::New(prop));


#endif