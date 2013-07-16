#ifndef MACROS_H
#define MACROS_H

// nodeUtil.h included ifndef XBLAB_NATIVE

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

#define XBGOOD 0
#define XBMAXCONCURRENT 128
#define XBEMITARGS 2
#define XBEMIT "emit"

#define XBEMITCALLBACK(h, a) node::MakeCallback(h, "emit", 2, a)

#ifndef XBLAB_NATIVE

#include "binding/nodeUtil.h"

#define XBGROUP "group"
#define XBUSERNAME "username"
#define XBPASSWORD "password"

#define GET_PROP(obj, setting) obj->Get(String::New(#setting)) 

// HACK - there may be a much nicer way to do this
#define THROW_FIELD_EX(prop) { stringstream ss;           \
      ss << "Unable to set value of readonly field \'"    \
      << xblab::NodeUtil::v8ToString(prop) << "\'";              \
      ThrowException(String::New(ss.str().c_str())); }

// Get node::Buffer constructor from JS land
#define JS_NODE_BUF_CTOR Persistent<Function>::New(Local<Function>:: \
      Cast(Context::GetCurrent()->Global()->Get(String::New("Buffer"))));     

#define THROW(prop) ThrowException(String::New(prop));

#else

#define YAJL_ERR_BUF_SZ 1024

/* property names must be identical to variable names */
#define GET_PROP(name) {                                      \
  const char * path[] = { #name, (const char *) 0 };          \
  yajl_val v = yajl_tree_get(node, path, yajl_t_string);      \
  if (v) {                                                    \
    name = YAJL_GET_STRING(v);                                \
  }                                                           \
  else {                                                      \
    printf("no such node: %s\n", path[0]);                    \
  }                                                           \
}

#define PRINT_YAJL_ERRBUF(errbuf) {           \
  fprintf(stderr, "parse_error: ");           \
  if (strlen(errbuf)) {                       \
    fprintf(stderr, " %s", errbuf);           \
  }                                           \
  else {                                      \
    fprintf(stderr, "unknown error");         \
  }                                           \
  fprintf(stderr, "\n");                      \
}

#endif

#define log(x) printf("%s\n", x);

#endif