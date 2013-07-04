#ifndef MACROS_H
#define MACROS_H

#ifndef XBLAB_NATIVE
#include "util.h"
#endif

// #define NDEBUG

#include <assert.h>

#define BITSIZE 2048
#define NONCE_SIZE 8
#define SIG_BUF_SIZE 4096
#define BCRYPT_WORK_FACTOR 12
#define BCRYPT_PH_SIZE 60

#define SHA1 "EMSA1(SHA-1)"
#define SHA256 "EME1(SHA-256)"
#define EMESHA1 "EME1(SHA-1)"
#define HMACSHA1 "HMAC(SHA-1)"
#define KDF2SHA1 "KDF2(SHA-1)"
#define CAST128 "CAST-128/CBC/PKCS7"
#define CAST "CAST"
#define MAC "MAC"
#define IV "IV"
#define CASTBYTES 16
#define MACBYTES 16
#define IVBYTES 8
#define MACOUTLEN 12


#ifndef XBLAB_NATIVE

// HACK - there may be a much nicer way to do this
#define THROW_FIELD_EX(prop) { stringstream ss;                 \
            ss << "Unable to set value of readonly field \'"    \
            << Util::v8ToString(prop) << "\'";                  \
            ThrowException(String::New(ss.str().c_str())); }

// Get node::Buffer constructor from JS land
#define JS_NODE_BUF_CTOR Persistent<Function>::New(Local<Function>::                \
            Cast(Context::GetCurrent()->Global()->Get(String::New("Buffer"))));     

#define THROW(prop) ThrowException(String::New(prop));

#endif


#endif